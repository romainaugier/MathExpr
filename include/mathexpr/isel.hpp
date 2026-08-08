// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHEXPR_ISEL)
#define __MATHEXPR_ISEL

#include "mathexpr/platform.hpp"
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