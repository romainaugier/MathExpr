// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#include "mathexpr/log.hpp"
#include "mathexpr/expr.hpp"

#include "../utils.hpp"

// "sin(a) + b" reads an undefined v4

int main(void)
{
    mathexpr::set_log_level(mathexpr::LogLevel::Debug);
    mathexpr::log_info("Starting call_result_statements_vreg test");

    const char* expression = "sin(a) + b";

    mathexpr::Expr expr(expression);

    if(!expr.compile(mathexpr::ExprPrintFlags::PrintAll))
    {
        mathexpr::log_error("Error while compiling expression");
        return 1;
    }

    double a = -1.0;

    auto [success, res] = expr.evaluate(a);

    if(!success)
    {
        mathexpr::log_error("Error during expression evaluation");
        return 1;
    }

    mathexpr::log_info("expr \"{}\" evaluated: ({}) = {}",
                       expression,
                       a,
                       res);

    if(!DOUBLE_EQ(res, 1.0))
        return 1;

    mathexpr::log_info("Finished call_result_statements_vreg test");

    return 0;
}
