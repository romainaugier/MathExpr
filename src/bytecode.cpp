// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#include "mathexpr/bytecode.hpp"

#include <ranges>

MATHEXPR_NAMESPACE_BEGIN

void bytecode_as_hex_string(const ByteCode& bytecode,
                            std::string& out,
                            const std::unordered_set<std::byte>& prefixes) noexcept
{
    out.clear();
    out.reserve(bytecode.size() * 2);

    for(std::size_t i = 0; i < bytecode.size(); i++)
    {
        if(i > 0 && prefixes.contains(bytecode[i]))
            std::format_to(std::back_inserter(out), "\n");

        std::format_to(std::back_inserter(out), "{}", bytecode[i]);
    }
}

MATHEXPR_NAMESPACE_END
