// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#include "mathexpr/mir.hpp"

#include <format>
#include <iterator>

MATHEXPR_NAMESPACE_BEGIN

std::string_view mir_op_to_string(MIROp op) noexcept
{
    switch(op)
    {
        case MIROp::FNeg: return "fneg";
        case MIROp::FAbs: return "fabs";
        case MIROp::FAdd: return "fadd";
        case MIROp::FSub: return "fsub";
        case MIROp::FMul: return "fmul";
        case MIROp::FDiv: return "fdiv";
        case MIROp::FMAdd: return "fmadd";
        case MIROp::FMSub: return "fmsub";
        case MIROp::FNMAdd: return "fnmadd";
        case MIROp::FNMSub: return "fnmsub";
        case MIROp::FAnd: return "fand";
        case MIROp::FAndNot: return "fandnot";
        case MIROp::FOr: return "for";
        case MIROp::FXor: return "fxor";
        case MIROp::Load: return "load";
        case MIROp::Store: return "store";
        case MIROp::Call: return "call";
        case MIROp::Move: return "move";
        case MIROp::Ret: return "ret";
        case MIROp::StackAlloc: return "stackalloc";
        case MIROp::StackFree: return "stackfree";
        default: return "unknown";
    }
}

std::string_view mir_memclass_to_string(MIRMemClass mem) noexcept
{
    switch(mem)
    {
        case MIRMemClass::Variables: return "variables";
        case MIRMemClass::Literals: return "literals";
        case MIRMemClass::Stack: return "stack";
        default: return "unknown";
    }
}

void mir_format_operand(std::ostream_iterator<char>& out, const MIROperand& operand) noexcept
{
    switch(operand.type)
    {
        case MIROperandType::VReg:
            std::format_to(out, "v{}", operand.vreg.id);
            break;

        case MIROperandType::PhysReg:
            std::format_to(out, "r{}", operand.vreg.id);
            break;

        case MIROperandType::Immediate:
            std::format_to(out, "#{}", operand.vreg.id);
            break;

        case MIROperandType::Memory:
        {
            switch(operand.mem.base)
            {
                case MIRMemClass::Variables:
                    std::format_to(out, "[vars + {}]", operand.mem.offset);
                    break;
                case MIRMemClass::Literals:
                    std::format_to(out, "[lits + {}]", operand.mem.offset);
                    break;
                case MIRMemClass::Stack:
                    std::format_to(out, "[stack + {}]", operand.mem.offset);
                    break;
            }

            break;
        }

        case MIROperandType::Func:
        {
            const libmaths::FunctionEntry* func = libmaths::get_function_entry(operand.func_id);

            if(func == nullptr)
                std::format_to(out, "invalid_func (id: {})", static_cast<std::uint32_t>(operand.func_id));
            else
                std::format_to(out, "{}", func->name);

            break;
        }

        default:
            break;
    }

    if(operand.type <= MIROperandType::PhysReg && operand.flags & MIROperand::Flags::Def)
        std::format_to(out, "<");
}

void MIRInstr::print(std::ostream_iterator<char>& out) const noexcept
{
    std::format_to(out,
                   "    {} ",
                   mir_op_to_string(this->op));

    for(std::uint32_t i = 0; i < this->num_operands; i++)
    {
        if(i > 0)
            std::format_to(out, ", ");

        mir_format_operand(out, this->operands[i]);
    }

    std::format_to(out, "\n");
}

void MIRFunc::print() const noexcept
{
    static std::ostream_iterator<char> out(std::cout);

    std::format_to(out, "MIRFunction\n");

    for(const auto& instr : this->instructions)
        instr.print(out);

    std::format_to(out, "\n");
}

MATHEXPR_NAMESPACE_END