// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHEXPR_SIMDTYPES)
#define __MATHEXPR_SIMDTYPES

#include "mathexpr/common.hpp"

#if defined(MATHEXPR_X86_64)
#include <immintrin.h>
#elif defined(MATHEXPR_AARCH64)
#include <arm_neon.h>
#endif // defined(MATHEXPR_X86_64)

MATHEXPR_NAMESPACE_BEGIN

#if defined(MATHEXPR_X86_64)
using double2 = __m128d;
using double4 = __m256d;
#elif defined(MATHEXPR_AARCH64)
using double2 = float64x2_t;
#endif // defined(MATHEXPR_X86_64)

MATHEXPR_NAMESPACE_END

#endif /* !defined(__MATHEXPR_SIMDTYPES) */
