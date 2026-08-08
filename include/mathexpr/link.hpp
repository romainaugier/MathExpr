// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHEXPR_LINK)
#define __MATHEXPR_LINK

#include "mathexpr/bytecode.hpp"

#include <string_view>
#include <vector>

MATHEXPR_NAMESPACE_BEGIN

enum RelocType : std::uint8_t
{
    RelocType_Rel32,
    RelocType_Abs64,
    RelocType_Abs4x16,
};

/* Information for instructions that need linking */
struct RelocInfo
{
    std::string_view symbol_name = "";      /* name of the symbol to link */
    std::size_t bytecode_offset = 0;        /* where to apply reloc in the bytecode */
    RelocType reloc_type = RelocType_Abs64; /* type of reloc */
    std::size_t instr_offset = 0;           /* offset between instructions to patch (for aarch64) */
};

using Relocations = std::vector<RelocInfo>;

MATHEXPR_API bool relocate(ByteCode& bytecode, const Relocations& relocations) noexcept;

MATHEXPR_NAMESPACE_END

#endif /* !defined(__MATHEXPR_LINK) */
