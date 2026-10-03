// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#include "mathexpr/log.hpp"
#include "mathexpr/expr.hpp"

#include "../utils.hpp"

// "sin(a) + b" reads an undefined v4
// "a*(b*(c*(d*(e*(f*(g*(h*(i*(j*(k*(l*(m*(n*(o*(p*(q*r))))))))))))))))"
// finalize uses r14 (on a SysV abi) as scratch for a spilled def
// in this case, o is gone
// example in MIR:
//     load r14<, [vars + 112]    ; RA put o in r14
//     load r15<, [vars + 120]
//     load r0<,  [vars + 128]
//     load r14<, [vars + 136]    ; finalize uses r14 as scratch for a spilled def so o is gone
//     store r14, [stack + 8]
//     ...
//     fmul r15<, r14, r0         ; meant to read o but reads something else

int main(void)
{
    mathexpr::set_log_level(mathexpr::LogLevel::Debug);
    mathexpr::log_info("Starting reserve_scratch_registers test");

    const char* expression = "a*(b*(c*(d*(e*(f*(g*(h*(i*(j*(k*(l*(m*(n*(o*(p*(q*r))))))))))))))))";

    mathexpr::Expr expr(expression);

    if(!expr.compile(mathexpr::ExprPrintFlags::PrintAll))
    {
        mathexpr::log_error("Error while compiling expression");
        return 1;
    }

    mathexpr::log_info("Finished reserve_scratch_registers test");

    return 0;
}
