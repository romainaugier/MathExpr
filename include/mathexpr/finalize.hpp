// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHEXPR_FINALIZE)
#define __MATHEXPR_FINALIZE

#include "mathexpr/platform.hpp"
#include "mathexpr/mir.hpp"
#include "mathexpr/abi.hpp"
#include "mathexpr/regalloc.hpp"

MATHEXPR_NAMESPACE_BEGIN

/*
    Per-round MIR finalization.

    Takes the MIR + the RA result and produces a concrete MIR where:
      - every vreg operand is replaced by a physreg or a stack slot
      - spills (Store after def) and reloads (Load before use) are
        materialized
      - memory-memory operands are resolved through scratch registers
      - the frame layout is computed (stack_size) and the
        prologue/epilogue (StackAlloc, callee-saved saves/restores,
        StackFree, Ret) is emitted
*/

struct FrameInfo
{
    std::uint32_t stack_size;        /* total, aligned */
    std::uint32_t spill_area_offset; /* offset of spill slot 0 */
    std::uint32_t slot_stride;       /* register width of the round */
};

MATHEXPR_API bool finalize_mir(const MIRFunc& in,
                               const RAFuncInfo& ra,
                               const PlatformABI* abi,
                               const ScalarType type,
                               MIRFunc& out) noexcept;

MATHEXPR_NAMESPACE_END

#endif // !defined(__MATHEXPR_FINALIZE)