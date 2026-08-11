// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHEXPR_TYPES)
#define __MATHEXPR_TYPES

#include "mathexpr/common.hpp"

#if defined(MATHEXPR_X86_64)
#include <immintrin.h>
#elif defined(MATHEXPR_AARCH64)
#include <arm_neon.h>
#endif // defined(MATHEXPR_X86_64)

MATHEXPR_NAMESPACE_BEGIN

enum class ScalarType : std::uint8_t
{
    F32 = 0,
    F64,
};

MATHEXPR_API std::size_t scalar_type_size(const ScalarType type) noexcept;

#if defined(MATHEXPR_X86_64)
using float4 = __m128;
using float8 = __m256;
using double2 = __m128d;
using double4 = __m256d;
#elif defined(MATHEXPR_AARCH64)
using float4 = float32x4_t;
using double2 = float64x2_t;
#endif // defined(MATHEXPR_X86_64)

MATHEXPR_NAMESPACE_END

#endif /* !defined(__MATHEXPR_TYPES) */
