// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#include "mathexpr/regalloc.hpp"

#include <algorithm>
#include <bitset>

MATHEXPR_NAMESPACE_BEGIN

struct LiveInterval
{
    std::uint32_t vreg;
    std::uint32_t start; // instruction index of the def
    std::uint32_t end; // instruction index of the last use
    bool spans_call;
};

struct FixedInterval
{
    std::uint32_t phys;
    std::uint32_t pos;
};

struct ActiveInterval
{
    std::uint32_t vreg;
    std::uint32_t end;
    PhysLocation loc;
};

constexpr std::size_t MAX_FP_REGS = 64;

bool RegAllocator::allocate(const MIRFunc& func,
                            const PlatformABI* abi,
                            RAFuncInfo& out) noexcept
{
    std::array<std::byte, 10_Kb> buffer;
    std::pmr::monotonic_buffer_resource pool{buffer.data(), buffer.size()};

    const std::size_t num_instrs = func.instructions.size();
    const std::size_t num_vregs = func.num_fp_vregs;

    // Build the live intervals
    std::pmr::vector<LiveInterval> intervals(num_vregs, std::addressof(pool));
    std::pmr::vector<FixedInterval> fixed(std::addressof(pool));

    for(std::uint32_t v = 0; v < num_vregs; ++v)
    {
        intervals[v].vreg = v;
        intervals[v].start = std::numeric_limits<std::uint32_t>::max();
        intervals[v].end = 0;
        intervals[v].spans_call = false;
    }

    // Find call instructions
    std::pmr::vector<std::uint32_t> call_points{std::addressof(pool)};

    for(std::uint32_t i = 0; i < num_instrs; ++i)
    {
        const MIRInstr& instr = func.instructions[i];

        if(instr.op == MIROp::Call)
            call_points.push_back(i);

        for(std::uint32_t o = 0; o < instr.num_operands; ++o)
        {
            const MIROperand& op = instr.operands[o];

            if(op.type == MIROperandType::PhysReg)
            {
                fixed.push_back({ op.physreg, i });
                continue;
            }
            else if(op.type != MIROperandType::VReg)
            {
                continue;
            }

            LiveInterval& iv = intervals[op.vreg.id];

            if(op.flags & MIROperand::Flags::Def)
            {
                iv.start = std::min(iv.start, i);
                iv.end = std::max(iv.end, i);
            }
            else if(op.flags & MIROperand::Flags::Use)
            {
                iv.end = std::max(iv.end, i);
            }
        }
    }

    for(LiveInterval& iv : intervals)
    {
        // Should never happen
        if(iv.start == std::numeric_limits<std::uint32_t>::max())
            continue;

        const auto it = std::lower_bound(call_points.begin(),
                                         call_points.end(), iv.start);

        iv.spans_call = (it != call_points.end() && *it <= iv.end);
    }

    // Linear scan
    const std::span<const std::uint32_t> volatile_regs = abi->get_caller_saved_fp_registers();
    const std::span<const std::uint32_t> callee_saved_regs = abi->get_callee_saved_fp_registers();

    std::bitset<MAX_FP_REGS> reg_free;
    reg_free.set();

    std::bitset<MAX_FP_REGS> callee_saved_used;

    std::pmr::vector<std::uint32_t> free_stack_slots{std::addressof(pool)};

    std::pmr::vector<ActiveInterval> active{std::addressof(pool)};
    active.reserve(num_vregs);

    out.locations.resize(num_vregs);
    out.num_spill_slots = 0;

    auto expire_old_intervals = [&](std::uint32_t pos) noexcept
    {
        for(auto it = active.begin(); it != active.end();)
        {
            if(it->end >= pos)
            {
                ++it;
                continue;
            }

            if(it->loc.is_reg)
                reg_free.set(it->loc.value);
            else
                free_stack_slots.push_back(it->loc.value);

            it = active.erase(it);
        }
    };

    auto alloc_stack_slot = [&]() noexcept -> std::uint32_t
    {
        if(!free_stack_slots.empty())
        {
            const std::uint32_t slot = free_stack_slots.back();
            free_stack_slots.pop_back();
            return slot;
        }

        return out.num_spill_slots++;
    };

    auto try_alloc_reg = [&](std::span<const std::uint32_t> regs,
                             std::uint32_t& out_reg) noexcept -> bool
    {
        for(const std::uint32_t r : regs)
        {
            if(r < MAX_FP_REGS && reg_free.test(r))
            {
                reg_free.reset(r);
                out_reg = r;
                return true;
            }
        }

        return false;
    };

    auto blocked = [&](std::uint32_t r, std::uint32_t start, std::uint32_t end)
    {
        for(const FixedInterval& f : fixed)
            if(f.phys == r && f.pos >= start && f.pos < end)
                return true;

        return false;
    };

    for(const LiveInterval& iv : intervals)
    {
        if(iv.start == std::numeric_limits<std::uint32_t>::max())
            continue;

        expire_old_intervals(iv.start);

        std::uint32_t reg = 0;
        bool allocated = false;

        if(iv.spans_call)
        {
            // Interval is live across a call: volatile registers are clobbered, only callee-saved ones can hold it
            // If none is free, spill (it's cheaper than save/restore around the call)
            if(try_alloc_reg(callee_saved_regs, reg))
            {
                callee_saved_used.set(reg);
                allocated = true;
            }

            if(blocked(reg, iv.start, iv.end))
                continue;
        }
        else
        {
            // No call in the interval: prefer volatile registers so we don't pay prologue/epilogue saves,
            // fall back to callee-saved, then spill
            if(try_alloc_reg(volatile_regs, reg))
            {
                allocated = true;
            }
            else if(try_alloc_reg(callee_saved_regs, reg))
            {
                callee_saved_used.set(reg);
                allocated = true;
            }
        }

        if(allocated)
        {
            out.locations[iv.vreg] = PhysLocation::reg(reg);
            active.push_back({ iv.vreg, iv.end, out.locations[iv.vreg] });
            continue;
        }

        // No register is available so we spill the interval that ends last (the current one, or one of the active ones).
        auto victim = std::max_element(active.begin(), active.end(), [](const ActiveInterval& a, const ActiveInterval& b) { 
            return a.end < b.end;
        });

        if(victim != active.end() && victim->end > iv.end)
        {
            // Steal the victim's register if it has one that fits our constraints otherwise we just take a stack slot ourselves
            const bool victim_reg_ok = victim->loc.is_reg && 
                                       (!iv.spans_call ||
                                        std::find(callee_saved_regs.begin(),
                                                  callee_saved_regs.end(),
                                                  victim->loc.value) != callee_saved_regs.end()
                                       );

            if(victim_reg_ok)
            {
                const PhysLocation stolen = victim->loc;

                victim->loc = PhysLocation::stack(alloc_stack_slot());
                out.locations[victim->vreg] = victim->loc;

                out.locations[iv.vreg] = stolen;
                active.push_back({ iv.vreg, iv.end, stolen });
            }
            else
            {
                out.locations[iv.vreg] = PhysLocation::stack(alloc_stack_slot());
                active.push_back({ iv.vreg, iv.end, out.locations[iv.vreg] });
            }
        }
        else
        {
            out.locations[iv.vreg] = PhysLocation::stack(alloc_stack_slot());
            active.push_back({ iv.vreg, iv.end, out.locations[iv.vreg] });
        }
    }

    // Record the callee-saved registers the prologue must save
    for(std::uint32_t r = 0; r < MAX_FP_REGS; ++r)
    {
        if(callee_saved_used.test(r))
            out.used_callee_saved.push_back(r);
    }

    return true;
}

MATHEXPR_NAMESPACE_END