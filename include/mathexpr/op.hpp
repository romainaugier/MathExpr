// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHEXPR_OP)
#define __MATHEXPR_OP

#include "mathexpr/common.hpp"

#include <string_view>

MATHEXPR_NAMESPACE_BEGIN

enum class UnaryOpType : std::uint32_t
{
    Unknown,
    Neg,
    Abs,
};

MATHEXPR_API const char* op_unary_to_string(const UnaryOpType type) noexcept;

MATHEXPR_API UnaryOpType op_unary_from_string(const std::string_view& data) noexcept;

enum class BinaryOpType : std::uint32_t 
{
    Unknown,
    Add,
    Sub,
    Mul,
    Div,
};

MATHEXPR_API const char* op_binary_to_string(const BinaryOpType type) noexcept;

MATHEXPR_API BinaryOpType op_binary_from_string(const std::string_view& data) noexcept;

MATHEXPR_API bool op_binary_is_commutative(const BinaryOpType type) noexcept;

MATHEXPR_NAMESPACE_END

#endif /* !defined(__MATHEXPR_OP) */