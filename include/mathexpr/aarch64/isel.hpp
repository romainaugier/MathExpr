// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHEXPR_AARCH64_ISEL)
#define __MATHEXPR_AARCH64_ISEL

#include "mathexpr/isel.hpp"

MATHEXPR_NAMESPACE_BEGIN

namespace aarch64
{

class MATHEXPR_API ISelAArch64 final : public ISel
{
protected:
    /* aarch64 has native fneg/fabs, single instruction */

    void emit_neg(MIRFunc& func,
                  std::uint32_t dst,
                  std::uint32_t src,
                  SymbolTable&,
                  ScalarType) const noexcept override
    {
        func.instructions.emplace_back(MIROp::FNeg,
            std::initializer_list<MIROperand>{
                MIROperand::vreg_def(dst),
                MIROperand::vreg_use(src),
            });
    }

    void emit_abs(MIRFunc& func,
                  std::uint32_t dst,
                  std::uint32_t src,
                  SymbolTable&,
                  ScalarType) const noexcept override
    {
        func.instructions.emplace_back(MIROp::FAbs,
            std::initializer_list<MIROperand>{
                MIROperand::vreg_def(dst),
                MIROperand::vreg_use(src),
            });
    }
};

} // namespace aarch64
MATHEXPR_NAMESPACE_END

#endif // !defined(__MATHEXPR_AARCH64_ISEL)