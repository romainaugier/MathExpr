// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHEXPR_MIR)
#define __MATHEXPR_MIR

#include "mathexpr/platform.hpp"
#include "mathexpr/ssa.hpp"
#include "mathexpr/utils/enumerate.hpp"
#include "mathexpr/libmaths.hpp"

#include <string>
#include <span>

MATHEXPR_NAMESPACE_BEGIN

/*
    MIR: machine-level intermediate representation
*/

enum class MIROp : std::uint32_t
{
    // Unary Ops
    FNeg,
    FAbs,

    // Binary ops
    FAdd, 
    FSub, 
    FMul, 
    FDiv,
    
    // Fused multiply-add/sub, not yet implemented but for future optimizations
    FMAdd, 
    FMSub, 
    FNMAdd, 
    FNMSub,

    // Bitwise ops
    FAnd, 
    FAndNot, 
    FOr, 
    FXor,

    // Memory (dst <- [base+offset] / [base+offset] <- src)
    Load,
    Store,

    // Control flow
    Call,      /* operands[0] = Func, followed by arg defs/vreg uses */
    Move,      /* physreg -> physreg (ABI moves, RA fixups) */
    Ret,

    // Stack allocation/free for frames and prologue/epilogue
    StackAlloc,
    StackFree,
};

enum class MIROperandType : std::uint8_t
{
    // Virtual register
    VReg,

    // Physical register
    PhysReg,

    // Immediate small integer (stack offset, memory offset)
    Immediate,

    // Memory (see MemClass)
    Memory,

    // FunctionId
    Func,
};

enum class MIRMemClass : std::uint8_t
{
    Variables,
    Literals,
    Stack,
};

struct MIROperand
{
    enum class Flags : std::uint8_t
    {
        None = 0,
        Def = 0x1,
        Use = 0x2,
    };

    MIROperandType type;
    Flags flags;

    union
    {
        struct { std::uint32_t id; } vreg;
        std::uint32_t physreg;
        std::int64_t imm;
        struct { MIRMemClass base; std::int32_t offset; } mem;
        libmaths::FunctionId func_id;
    };

    static MIROperand vreg_def(std::uint32_t id) noexcept
    {
        MIROperand o{};
        o.type = MIROperandType::VReg;
        o.flags = Flags::Def;
        o.vreg.id = id;
        return o;
    }

    static MIROperand vreg_use(std::uint32_t id) noexcept
    {
        MIROperand o = vreg_def(id);
        o.flags = Flags::Use;
        return o;
    }

    static MIROperand phys(std::uint32_t reg, Flags flags) noexcept
    {
        MIROperand o{};
        o.type = MIROperandType::PhysReg;
        o.flags = flags;
        o.physreg = reg;
        return o;
    }

    static MIROperand immediate(std::int64_t v) noexcept
    {
        MIROperand o{};
        o.type = MIROperandType::Immediate;
        o.flags = Flags::None;
        o.imm = v;
        return o;
    }

    static MIROperand memory(MIRMemClass base, std::int32_t offset) noexcept
    {
        MIROperand o{};
        o.type = MIROperandType::Memory;
        o.flags = Flags::None;
        o.mem.base = base;
        o.mem.offset = offset;
        return o;
    }

    static MIROperand function(libmaths::FunctionId func_id) noexcept
    {
        MIROperand o{};
        o.type = MIROperandType::Func;
        o.flags = Flags::None;
        o.func_id = func_id;
        return o;
    }
};

MATHEXPR_FORCE_INLINE bool operator&(const MIROperand::Flags left,
                                     const MIROperand::Flags right) noexcept
{
    return (static_cast<std::uint8_t>(left) & static_cast<std::uint8_t>(right)) > 0;
}

struct MIRInstr
{
    static constexpr std::size_t MAX_OPERANDS = 5;

    MIROp op;
    std::uint32_t num_operands;
    std::array<MIROperand, MAX_OPERANDS> operands;

    explicit MIRInstr(MIROp op, std::initializer_list<MIROperand> operands) noexcept
    {
        this->op = op;
        this->num_operands = static_cast<std::uint32_t>(operands.size());

        for(const auto& [i, operand] : enumerate(operands))
            this->operands[i] = operand;
    }

    void print(std::ostream_iterator<char>& out) const noexcept;
};

struct MATHEXPR_API MIRFunc
{
    std::uint32_t num_fp_vregs = 0;

    /* frame, computed by the register allocator */
    std::uint32_t stack_size = 0;
    std::uint32_t num_spill_slots = 0;

    std::vector<MIRInstr> instructions;
    std::vector<std::uint32_t> call_clobbered;

    std::uint32_t create_fp_vreg() noexcept { return this->num_fp_vregs++; }

    void add_call_clobbers(std::span<const std::uint32_t> clobbers)
    {
        this->call_clobbered.insert(this->call_clobbered.end(),
                                    clobbers.begin(),
                                    clobbers.end());
    }

    void print() const noexcept;
};

/*
    Debug print
*/

MATHEXPR_NAMESPACE_END

#endif // !defined(__MATHEXPR_MIR)