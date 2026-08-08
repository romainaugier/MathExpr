// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHEXPR_X86_64_ISEL)
#define __MATHEXPR_X86_64_ISEL

#include "mathexpr/isel.hpp"
#include "mathexpr/x86_64/x86_64.hpp"

MATHEXPR_NAMESPACE_BEGIN

X86_64_NAMESPACE_BEGIN

class MATHEXPR_API ISelX86_64 final : public ISel
{
    static constexpr std::string_view SIGN_BIT_LITERAL_NAME = "sign_bit_literal";
    static constexpr double SIGN_BIT_LITERAL = std::bit_cast<double>(0x8000000000000000);

    static constexpr std::string_view NOT_SIGN_BIT_LITERAL_NAME = "not_sign_bit_literal";
    static constexpr double NOT_SIGN_BIT_LITERAL = std::bit_cast<double>(0x7FFFFFFFFFFFFFFF);
protected:
    /*
        x86 has no fneg/fabs: expand to xor/and with a sign mask
        literal. The emitter will pick xorps/xorpd (or the packed
        variants) depending on the compilation round.
    */

    void emit_neg(MIRFunc& func,
                  std::uint32_t dst,
                  std::uint32_t src,
                  SymbolTable& symtab,
                  ScalarType st) const noexcept override
    {
        const std::size_t offset = symtab.add_literal(SIGN_BIT_LITERAL_NAME,
                                                      SIGN_BIT_LITERAL);

        const std::uint32_t tmp = func.create_fp_vreg();

        this->emit_load_literal(func, tmp, offset);

        func.instructions.emplace_back(MIROp::FXor,
            std::initializer_list<MIROperand>{
                MIROperand::vreg_def(dst),
                MIROperand::vreg_use(src),
                MIROperand::vreg_use(tmp),
            });
    }

    void emit_abs(MIRFunc& func,
                  std::uint32_t dst,
                  std::uint32_t src,
                  SymbolTable& symtab,
                  ScalarType st) const noexcept override
    {
        const std::uint32_t offset = symtab.add_literal(NOT_SIGN_BIT_LITERAL_NAME,
                                                        NOT_SIGN_BIT_LITERAL);

        const std::uint32_t tmp = func.create_fp_vreg();

        this->emit_load_literal(func, tmp, offset);

        func.instructions.emplace_back(MIROp::FAnd,
            std::initializer_list<MIROperand>{
                MIROperand::vreg_def(dst),
                MIROperand::vreg_use(src),
                MIROperand::vreg_use(tmp),
            });
    }
};

X86_64_NAMESPACE_END

MATHEXPR_NAMESPACE_END

#endif // !defined(__MATHEXPR_X86_64_ISEL)