// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHEXPR_MEMORY)
#define __MATHEXPR_MEMORY

#include "mathexpr/common.hpp"

MATHEXPR_NAMESPACE_BEGIN

constexpr std::size_t operator""_Kb(unsigned long long x) noexcept { return x * 1'024; }
constexpr std::size_t operator""_Mb(unsigned long long x) noexcept { return x * 1'048'576; }
constexpr std::size_t operator""_Gb(unsigned long long x) noexcept { return x * 1'073'741'824; }

MATHEXPR_NAMESPACE_END

#endif // !defined(__MATHEXPR_MEMORY)