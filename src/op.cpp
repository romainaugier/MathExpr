// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#include "mathexpr/op.hpp"

MATHEXPR_NAMESPACE_BEGIN

const char* op_unary_to_string(const UnaryOpType type) noexcept
{
    switch(type)
    {
        case UnaryOpType::Neg:
            return "-";
        case UnaryOpType::Abs:
            return "abs ";
        default:
            return "?";
    }
}

UnaryOpType op_unary_from_string(const std::string_view& data) noexcept
{
    MATHEXPR_ASSERT(data.size() >= 1, "data must contain at least one character");

    if(data == "-")
        return UnaryOpType::Neg;

    if(data == "abs")
        return UnaryOpType::Abs;

    return UnaryOpType::Unknown;
}

const char* op_binary_to_string(const BinaryOpType type) noexcept
{
    switch(type)
    {
        case BinaryOpType::Add: 
            return "+";
        case BinaryOpType::Sub: 
            return "-";
        case BinaryOpType::Mul: 
            return "*";
        case BinaryOpType::Div: 
            return "/";
        default:
            return "?";
    }
}

BinaryOpType op_binary_from_string(const std::string_view& data) noexcept
{
    MATHEXPR_ASSERT(data.size() >= 1, "data must contain at least one character");

    switch(data[0])
    {
        case '+': 
            return BinaryOpType::Add;
        case '-': 
            return BinaryOpType::Sub;
        case '*': 
            return BinaryOpType::Mul;
        case '/': 
            return BinaryOpType::Div;
        default:
            return BinaryOpType::Unknown;
    }
}

bool op_binary_is_commutative(const BinaryOpType type) noexcept
{
    switch(type)
    {
        case BinaryOpType::Add:
        case BinaryOpType::Mul:
            return true;

        default:
            return false;
    }
}

MATHEXPR_NAMESPACE_END