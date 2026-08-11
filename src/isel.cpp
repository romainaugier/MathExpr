// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#include "mathexpr/log.hpp"
#include "mathexpr/isel.hpp"
#include "mathexpr/x86_64/isel.hpp"
#include "mathexpr/aarch64/isel.hpp"

MATHEXPR_NAMESPACE_BEGIN

bool ISel::lower_ssa_to_mir(const SSA& ssa,
                            SymbolTable& symtab,
                            const PlatformABI* abi,
                            ScalarType st,
                            MIRFunc& out) const noexcept
{
    /* stmt -> vreg mapping */
    std::unordered_map<const SSAStmt*, std::uint32_t> vregs;
    vregs.reserve(ssa.get_statements().size());

    auto vreg_of = [&](const SSAStmt* stmt) noexcept -> std::uint32_t
    {
        auto it = vregs.find(stmt);

        if(it != vregs.end())
            return it->second;

        const std::uint32_t v = out.create_fp_vreg();
        vregs.emplace(stmt, v);
        
        return v;
    };

    for(const SSAStmt* stmt : ssa.get_statements())
    {
        switch(stmt->type_id())
        {
            case SSAStmtTypeId_Variable:
            {
                const auto* var = static_cast<const SSAStmtVariable*>(stmt);

                const std::size_t offset = symtab.get_variable_offset(var->get_name());

                if(offset == INVALID_OFFSET)
                {
                    log_error("Invalid variable name: {}", var->get_name());
                    return false;
                }

                out.instructions.emplace_back(MIROp::Load,
                    std::initializer_list<MIROperand>{
                        MIROperand::vreg_def(vreg_of(stmt)),
                        MIROperand::memory(MIRMemClass::Variables,
                                           static_cast<std::int32_t>(offset)),
                    });
                break;
            }

            case SSAStmtTypeId_Literal:
            {
                const auto* lit = static_cast<const SSAStmtLiteral*>(stmt);

                const std::size_t offset = symtab.get_literal_offset(lit->get_name());

                if(offset == INVALID_OFFSET)
                {
                    log_error("Invalid literal name: {}", lit->get_name());
                    return false;
                }

                this->emit_load_literal(out, vreg_of(stmt), offset);

                break;
            }

            case SSAStmtTypeId_UnOp:
            {
                const auto* unop = static_cast<const SSAStmtUnOp*>(stmt);

                const std::uint32_t src = vreg_of(unop->get_operand());
                const std::uint32_t dst = vreg_of(stmt);

                switch(unop->get_op())
                {
                    case UnaryOpType::Neg:
                        this->emit_neg(out, dst, src, symtab, st);
                        break;
                    case UnaryOpType::Abs:
                        this->emit_abs(out, dst, src, symtab, st);
                        break;
                    default:
                        return false;
                }
                break;
            }

            case SSAStmtTypeId_BinOp:
            {
                const auto* binop = static_cast<const SSAStmtBinOp*>(stmt);

                MIROp op;

                switch(binop->get_op())
                {
                    case BinaryOpType::Add: op = MIROp::FAdd; break;
                    case BinaryOpType::Sub: op = MIROp::FSub; break;
                    case BinaryOpType::Mul: op = MIROp::FMul; break;
                    case BinaryOpType::Div: op = MIROp::FDiv; break;
                    default: return false;
                }

                out.instructions.emplace_back(op,
                    std::initializer_list<MIROperand>{
                        MIROperand::vreg_def(vreg_of(stmt)),
                        MIROperand::vreg_use(vreg_of(binop->get_left())),
                        MIROperand::vreg_use(vreg_of(binop->get_right())),
                    });
                break;
            }

            case SSAStmtTypeId_FuncOp:
            {
                const auto* funcop = static_cast<const SSAStmtFunctionOp*>(stmt);

                std::vector<std::uint32_t> arg_vregs;
                arg_vregs.reserve(funcop->get_arguments().size());

                for(const SSAStmt* arg : funcop->get_arguments())
                    arg_vregs.emplace_back(vreg_of(arg));

                if(!ISel::emit_call(out, funcop, arg_vregs, abi))
                    return false;

                const libmaths::FunctionId func_id = libmaths::get_function_id(funcop->get_name());

                if(func_id == libmaths::FunctionId::Unknown)
                    return false;

                break;
            }

            default:
                return false;
        }
    }

    const MIRInstr& last_instr = out.instructions.back();

    if(last_instr.num_operands > 0)
    {
        out.instructions.emplace_back(MIROp::Move,
            std::initializer_list<MIROperand>{
                MIROperand::phys(abi->get_call_return_value_fp_register(), MIROperand::Flags::Def),
                MIROperand::vreg_use(last_instr.operands.front().vreg.id),
        });
    }

    out.instructions.emplace_back(MIROp::Ret);

    return true;
}

const ISel* get_isel(ISA isa) noexcept
{
    switch(isa)
    {
        case ISA::x86_64:
        {
            static const x86_64::ISelX86_64 isel;
            return std::addressof(isel);
        }

        case ISA::aarch64:
        {
            static const aarch64::ISelAArch64 isel;
            return &isel;
        }

        default:
            return nullptr;
    }
}

MATHEXPR_NAMESPACE_END