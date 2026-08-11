// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHEXPR_RA)
#define __MATHEXPR_RA

#include "mathexpr/platform.hpp"
#include "mathexpr/mir.hpp"
#include "mathexpr/abi.hpp"

#include <vector>

MATHEXPR_NAMESPACE_BEGIN

/*
    Linear scan register allocator, working on the MIR

    The result is a location per virtual register (physical register
    or stack slot). Spill/reload code is not inserted into the MIR:
    the emitter materializes a store after a def and a load before
    a use when the location is a stack slot. This keeps the MIR
    untouched and lets each round redo allocation from scratch.
*/

struct PhysLocation
{
    std::uint32_t value : 31; /* Physreg id, or stack slot index */
    bool is_reg : 1;

    static PhysLocation reg(std::uint32_t r) noexcept { return { r, true }; }
    static PhysLocation stack(std::uint32_t s) noexcept { return { s, false }; }
};

struct RAFuncInfo
{
private:
    static constexpr std::size_t FUNC_INFO_BUFFER_SZ = 128;

    std::array<std::byte, sizeof(std::uint32_t) * FUNC_INFO_BUFFER_SZ> buffer; 
    std::pmr::monotonic_buffer_resource pool{buffer.data(), buffer.size()};

public:
    RAFuncInfo() = default;

    MATHEXPR_NON_COPYABLE(RAFuncInfo);
    MATHEXPR_NON_MOVABLE(RAFuncInfo);

    /* Indexed by the vreg id */
    std::pmr::vector<PhysLocation> locations{std::addressof(pool)};

    /* Callee-saved fp registers used, must be saved/restored in the prologue/epilogue */
    std::pmr::vector<std::uint32_t> used_callee_saved{std::addressof(pool)};

    /* High-water mark of concurrently live spill slots */
    std::uint32_t num_spill_slots = 0;
};

class MATHEXPR_API RegAllocator
{
public:
    static bool allocate(const MIRFunc& func,
                         const PlatformABI* abi,
                         RAFuncInfo& out) noexcept;
};

MATHEXPR_NAMESPACE_END

#endif // !defined(__MATHEXPR_RA)