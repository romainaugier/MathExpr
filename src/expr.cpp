// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#include "mathexpr/expr.hpp"
#include "mathexpr/log.hpp"
#include "mathexpr/isel.hpp"
#include "mathexpr/codegen.hpp"
#include "mathexpr/regalloc.hpp"

#include <iterator>
#include <algorithm>

MATHEXPR_NAMESPACE_BEGIN

std::tuple<bool, double> Expr::_evaluate_internal(const double* values) const noexcept
{
    MATHEXPR_ASSERT(values != nullptr, "values is NULL");

    if(!this->_exec_mem.is_locked())
    {
        log_error("ExecMem is not locked nor ready, compile expr before evaluating it");
        return std::make_tuple(false, 0.0);
    }

    auto exec_func = this->_exec_mem.as_function();

    double result = exec_func(values, this->_literals.data());

    return std::make_tuple(true, result);
}

bool Expr::compile(ExprPrintFlags debug_flags) noexcept
{
    Platform platform = get_current_platform();

    if(platform == Platform::Invalid)
    {
        log_error("Current platform is not supported ({})", platform_as_string(platform));
        return false;
    }

    ISA isa = get_current_isa();

    if(isa == ISA::Invalid)
    {
        log_error("Current isa is not supported ({})", isa_as_string(isa));
        return false;
    }

    const PlatformABI* platform_abi = get_platform_abi(isa, platform);

    if(platform_abi == nullptr)
    {
        log_error("Current ABI is not supported ({})", platform_abi->get_as_string());
        return false;
    }

    SlabAllocator allocator{10_Kb};

    this->_variables.clear();
    this->_literals.clear();

    log_debug("Compiling expression: {}", this->_expr);

    auto [lex_success, tokens] = lexer_lex_expression(this->_expr);

    if(!lex_success)
    {
        log_error("Error while lexing expression: {}", this->_expr);
        log_error("Check the log for more information");
        return false;
    }

    AST ast(allocator);

    if(!ast.build_from_tokens(tokens))
    {
        log_error("Error while building AST for expression: {}", this->_expr);
        log_error("Check the log for more information");
        return false;
    }

    if(debug_flags & ExprPrintFlags::PrintAST)
        ast.print();

    SymbolTable symtable;

    symtable.collect(ast);

    if(debug_flags & ExprPrintFlags::PrintSymTable)
        symtable.print();

    /* Variables and literals are stored in order of parsing */
    for(auto [name, _] : symtable.get_variables())
        this->_variables.insert(name);

    for(auto [_, lit] : symtable.get_literals())
        this->_literals.push_back(lit.get_value());

    SSA ssa(allocator);

    if(!ssa.build_from_ast(ast))
    {
        log_error("Error while building SSA for expression: {}", this->_expr);
        log_error("Check the log for more information");
        return false;
    }

    if(debug_flags & ExprPrintFlags::PrintSSA)
        ssa.print();

    const ISel* isel = get_isel(isa);

    if(isel == nullptr)
    {
        log_error("Cannot find an Instruction Selector for current isa ({})",
                  isa_as_string(isa));
        return false;
    }

    MIRFunc mir_func;

    if(!isel->lower_ssa_to_mir(ssa, symtable, ScalarType::F64, mir_func))
    {
        log_error("Error while selecting instructions for expression: {}", this->_expr);
        log_error("Check the log for more information");
        return false;
    }

    mir_func.print();

    return false;
}

MATHEXPR_NAMESPACE_END
