// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHEXPR_REGALLOC)
#define __MATHEXPR_REGALLOC

#include "mathexpr/ssa.hpp"
#include "mathexpr/symtable.hpp"
#include "mathexpr/abi.hpp"

#include <unordered_map>

MATHEXPR_NAMESPACE_BEGIN

enum MemLocTypeId : uint32_t
{
    MemLocTypeId_Invalid,
    MemLocTypeId_Register,
    MemLocTypeId_Stack,
    MemLocTypeId_Memory,
};

enum MemLocRegister : uint32_t
{
    MemLocRegister_Variables,
    MemLocRegister_Literals,
};

struct MATHEXPR_API MemLoc
{
    enum class Kind : uint8_t { Invalid, Register, Stack, Memory };

    Kind kind = Kind::Invalid;
    RegisterId reg = INVALID_FP_REGISTER; // Register or base ptr form Memory
    uint64_t offset = 0;                  // Stack/Memory offset
};

MATHEXPR_FORCE_INLINE MemLoc memloc_register(RegisterId reg) noexcept
{
    return { .kind = MemLoc::Kind::Register, .reg = reg };
}

MATHEXPR_FORCE_INLINE MemLoc memloc_stack(uint64_t offset) noexcept
{
    return { .kind = MemLoc::Kind::Stack, .offset = offset };
}

MATHEXPR_FORCE_INLINE MemLoc memloc_memory(RegisterId base_ptr, uint64_t offset) noexcept
{
    return { .kind = MemLoc::Kind::Memory, .reg = base_ptr, .offset = offset };
}

class MATHEXPR_API RegisterAllocator
{
    std::unordered_map<SSAStmtPtr, MemLoc> _mapping;

    PlatformABIPtr _platform_abi;

    static bool prepass_commutative_operand_swap(SSA& ssa) noexcept;

    MemLoc get_reusable_register(const SSAStmtPtr& statement) const noexcept;

public:
    RegisterAllocator(PlatformABIPtr platform_abi) : _platform_abi(platform_abi) {}

    bool allocate(SSA& ssa, const SymbolTable& symtable) noexcept;

    MemLoc get_memloc(SSAStmtPtr& stmt) const noexcept
    {
        static MemLoc invalid;

        auto it = this->_mapping.find(stmt);

        return it != this->_mapping.end() ? it->second : invalid;
    }
};

MATHEXPR_NAMESPACE_END

#endif /* !defined(__MATHEXPR_REGALLOC) */
