// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHEXPR_ISEL)
#define __MATHEXPR_ISEL

#include "mathexpr/abi.hpp"
#include "mathexpr/log.hpp"
#include "mathexpr/mir.hpp"
#include "mathexpr/ssa.hpp"
#include "mathexpr/symtable.hpp"

MATHEXPR_NAMESPACE_BEGIN

/*
    ISel: instruction selector.

    Lowers SSA to MIR. The generic base performs the whole walk
    (statement -> vreg mapping, loads, binops, calls); each ISA
    overrides only the lowering hooks that differ between targets
    (neg/abs expansions, and later FMA contraction, scheduling...).
*/

class MATHEXPR_API ISel
{
public:
    virtual ~ISel() = default;

    bool lower_ssa_to_mir(const SSA& ssa,
                          SymbolTable& symtab,
                          const PlatformABI* abi,
                          ScalarType scalar_type,
                          MIRFunc& out) const noexcept;

protected:
    /*
        Lowering hooks, the only per-ISA differences for now
    */
    virtual void emit_neg(MIRFunc& func,
                          std::uint32_t dst,
                          std::uint32_t src,
                          SymbolTable& symtab,
                          ScalarType st) const noexcept = 0;

    virtual void emit_abs(MIRFunc& func,
                          std::uint32_t dst,
                          std::uint32_t src,
                          SymbolTable& symtab,
                          ScalarType st) const noexcept = 0;

    /* shared helpers */

    static bool emit_call(MIRFunc& func,
                          const SSAStmtFunctionOp* call,
                          std::span<const std::uint32_t> arg_vregs,
                          const PlatformABI* abi) noexcept
    {
        const auto arg_regs = abi->get_call_args_fp_registers();

        std::array<MIROperand, MIRInstr::MAX_OPERANDS> ops{};

        libmaths::FunctionId call_id = libmaths::get_function_id(call->get_name());

        if(call_id == libmaths::FunctionId::Unknown)
        {
            log_error("Unknown function: \"{}\"", call->get_name());
            return false;
        }

        ops[0] = MIROperand::function(call_id);

        for(std::size_t i = 0; i < call->get_arguments().size(); ++i)
        {
            // arg vreg must land in the ABI arg register
            func.instructions.emplace_back(MIROp::Move,
                std::initializer_list<MIROperand>{
                    MIROperand::phys(arg_regs[i], MIROperand::Flags::Def),
                    MIROperand::vreg_use(arg_vregs[i]),
                });

            ops[i + 1] = MIROperand::phys(arg_regs[i], MIROperand::Flags::Use);
        }

        func.instructions.emplace_back(MIROp::Call, ops.begin(), ops.begin() + call->get_arguments().size() + 1);

        // Call result comes back in the return register
        const std::uint32_t dst = func.create_fp_vreg();

        func.instructions.emplace_back(MIROp::Move,
                                       std::initializer_list<MIROperand>{
            MIROperand::vreg_def(dst),
            MIROperand::phys(abi->get_call_return_value_fp_register(), MIROperand::Flags::Use),
        });

        return true;
    }

    static void emit_load_variable(MIRFunc& func,
                                   std::uint32_t dst,
                                   std::uint32_t offset) noexcept
    {
        func.instructions.emplace_back(MIROp::Load,
            std::initializer_list<MIROperand>{
                MIROperand::vreg_def(dst),
                MIROperand::memory(MIRMemClass::Variables,
                                   static_cast<std::int32_t>(offset)),
            });
    }

    static void emit_load_literal(MIRFunc& func,
                                  std::uint32_t dst,
                                  std::uint32_t offset) noexcept
    {
        func.instructions.emplace_back(MIROp::Load,
            std::initializer_list<MIROperand>{
                MIROperand::vreg_def(dst),
                MIROperand::memory(MIRMemClass::Literals,
                                   static_cast<std::int32_t>(offset)),
            });
    }
};

/* Returns the static selector for the given isa */
MATHEXPR_API const ISel* get_isel(ISA isa = get_current_isa()) noexcept;

MATHEXPR_NAMESPACE_END

#endif // !defined(__MATHEXPR_ISEL)