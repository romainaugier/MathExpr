// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#include "mathexpr/log.hpp"
#include "mathexpr/expr.hpp"

#include "../utils.hpp"

// "a + sin(b) * a" stores to [stack + 0] with no frame

int main(void)
{
    mathexpr::set_log_level(mathexpr::LogLevel::Debug);
    mathexpr::log_info("Starting allocator_skipping_intervals test");

    const char* expression = "a + sin(b) * a";

    mathexpr::Expr expr(expression);

    if(!expr.compile(mathexpr::ExprPrintFlags::PrintAll))
    {
        mathexpr::log_error("Error while compiling expression");
        return 1;
    }

    double a = 1.0;
    double b = 1.0;

    auto [success, res] = expr.evaluate(a, b);

    if(!success)
    {
        mathexpr::log_error("Error during expression evaluation");
        return 1;
    }

    mathexpr::log_info("expr \"{}\" evaluated: ({}, {}) = {}",
                       expression,
                       a,
                       b,
                       res);

    if(!DOUBLE_EQ(res, 1.0))
        return 1;

    mathexpr::log_info("Finished allocator_skipping_intervals test");

    return 0;
}

