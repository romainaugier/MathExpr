// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#include "mathexpr/finalize.hpp"

#include <algorithm>

MATHEXPR_NAMESPACE_BEGIN


constexpr std::size_t NUM_SCRATCH = 2;

// frame layout:
// [callee-saved saves][spill slots]

FrameInfo compute_frame(const RAFuncInfo& ra,
                        const PlatformABI* abi,
                        const ScalarType type) noexcept
{
    FrameInfo f{};

    // TODO: replace with current float width
    f.slot_stride = sizeof(double);

    const std::uint32_t save_bytes = static_cast<std::uint32_t>(ra.used_callee_saved.size()) * f.slot_stride;

    f.spill_area_offset = save_bytes;

    const std::uint32_t raw = save_bytes + ra.num_spill_slots * f.slot_stride;

    f.stack_size = (raw + (abi->stack_alignment() - 1)) & ~(abi->stack_alignment() - 1);

    return f;
}

bool finalize_mir(const MIRFunc& in,
                  const RAFuncInfo& ra,
                  const PlatformABI* abi,
                  const ScalarType type,
                  MIRFunc& out) noexcept
{
    const FrameInfo frame = compute_frame(ra, abi, type);

    const auto volatile_all = abi->get_caller_saved_fp_registers();
    const std::uint32_t scratch0 = volatile_all[volatile_all.size() - 2];
    const std::uint32_t scratch1 = volatile_all[volatile_all.size() - 1];

    // No vregs should be left after finalization
    out.num_fp_vregs = 0;
    out.stack_size = frame.stack_size;
    out.num_spill_slots = ra.num_spill_slots;
    out.instructions.clear();
    out.call_clobbered = in.call_clobbered;

    auto slot_offset = [&](std::uint32_t slot) noexcept -> std::int32_t
    {
        return static_cast<std::int32_t>(frame.spill_area_offset
                                       + slot * frame.slot_stride);
    };

    // Prologue: allocate the frame, then save the callee-saved registers the RA used (slot order = order in used_callee_saved)
    if(frame.stack_size > 0)
    {
        out.instructions.emplace_back(MIROp::StackAlloc,
            std::initializer_list<MIROperand>{
                MIROperand::immediate(frame.stack_size),
            });

        for(std::size_t i = 0; i < ra.used_callee_saved.size(); ++i)
        {
            out.instructions.emplace_back(MIROp::Store,
                std::initializer_list<MIROperand>{
                    MIROperand::phys(ra.used_callee_saved[i],
                                     MIROperand::Flags::Use),
                    MIROperand::memory(MIRMemClass::Stack,
                                       static_cast<std::int32_t>(i * frame.slot_stride)),
                });
        }
    }

    for(const MIRInstr& instr : in.instructions)
    {
        // Skip ret because we will add it after the stackfree 
        if(instr.op == MIROp::Ret)
            continue;

        // We collect the reloads to insert before this instruction, and the spill store to insert after
        std::array<std::uint32_t, MIRInstr::MAX_OPERANDS> reload_phys{};
        std::array<std::int32_t, MIRInstr::MAX_OPERANDS> reload_slot{};

        std::uint32_t num_reloads = 0;

        std::uint32_t def_phys = 0;
        std::int32_t def_slot = 0;

        bool has_def_spill = false;

        MIRInstr rewritten = instr;

        for(std::uint32_t o = 0; o < instr.num_operands; ++o)
        {
            const MIROperand& op = instr.operands[o];

            if(op.type != MIROperandType::VReg)
                continue;

            const PhysLocation loc = ra.locations[op.vreg.id];

            if(op.flags & MIROperand::Flags::Def)
            {
                if(loc.is_reg)
                {
                    rewritten.operands[o] = MIROperand::phys(loc.value, MIROperand::Flags::Def);
                }
                else
                {
                    /*
                        def to a slot: compute into scratch0, then
                        store to the slot after the instruction
                    */
                    rewritten.operands[o] =
                        MIROperand::phys(scratch0, MIROperand::Flags::Def);
                    has_def_spill = true;
                    def_phys = scratch0;
                    def_slot = slot_offset(loc.value);
                }
            }
            else /* Use */
            {
                if(loc.is_reg)
                {
                    rewritten.operands[o] =
                        MIROperand::phys(loc.value, MIROperand::Flags::Use);
                }
                else
                {
                    /*
                        reload into a scratch reg (never scratch0 while
                        it's holding a spilled def -- use scratch1 when
                        the def was spilled and this is the 2nd+ operand)
                    */
                    const std::uint32_t scratch =
                        (has_def_spill && num_reloads > 0) ? scratch1 : scratch0;

                    rewritten.operands[o] =
                        MIROperand::phys(scratch, MIROperand::Flags::Use);

                    reload_phys[num_reloads] = scratch;
                    reload_slot[num_reloads] = slot_offset(loc.value);
                    ++num_reloads;
                }
            }
        }

        /* reloads first */
        for(std::uint32_t r = 0; r < num_reloads; ++r)
        {
            out.instructions.emplace_back(MIROp::Load,
                std::initializer_list<MIROperand>{
                    MIROperand::phys(reload_phys[r], MIROperand::Flags::Def),
                    MIROperand::memory(MIRMemClass::Stack, reload_slot[r]),
                });
        }

        out.instructions.push_back(rewritten);

        /* spill store after the def */
        if(has_def_spill)
        {
            out.instructions.emplace_back(MIROp::Store,
                std::initializer_list<MIROperand>{
                    MIROperand::phys(def_phys, MIROperand::Flags::Use),
                    MIROperand::memory(MIRMemClass::Stack, def_slot),
                });
        }
    }

    // Write epilogue: restore callee-saved, free the frame, return
    if(frame.stack_size > 0)
    {
        for(std::size_t i = 0; i < ra.used_callee_saved.size(); ++i)
        {
            out.instructions.emplace_back(MIROp::Load,
                std::initializer_list<MIROperand>{
                    MIROperand::phys(ra.used_callee_saved[i],
                                     MIROperand::Flags::Def),
                    MIROperand::memory(MIRMemClass::Stack,
                                       static_cast<std::int32_t>(i * frame.slot_stride)),
                });
        }

        out.instructions.emplace_back(MIROp::StackFree,
            std::initializer_list<MIROperand>{
                MIROperand::immediate(frame.stack_size),
            });
    }

    out.instructions.emplace_back(MIROp::Ret, std::initializer_list<MIROperand>{});

    return true;
}

MATHEXPR_NAMESPACE_END