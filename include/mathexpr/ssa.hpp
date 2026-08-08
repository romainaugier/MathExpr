// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHEXPR_SSA)
#define __MATHEXPR_SSA

#include "mathexpr/ast.hpp"
#include "mathexpr/platform.hpp"

MATHEXPR_NAMESPACE_BEGIN

enum SSAStmtTypeId : int
{
    SSAStmtTypeId_Variable = 1,
    SSAStmtTypeId_Literal = 2,
    SSAStmtTypeId_UnOp = 3,
    SSAStmtTypeId_BinOp = 4,
    SSAStmtTypeId_FuncOp = 5,
};

static constexpr char VERSION_CHAR = 't';

static constexpr uint64_t INVALID_STMT_VERSION = std::numeric_limits<uint64_t>::max();
static constexpr uint64_t INVALID_STMT_REGISTER = std::numeric_limits<uint64_t>::max();

class MATHEXPR_API SSAStmt
{
    uint64_t _version;

    uint64_t _frequency;

public:
    SSAStmt(std::uint64_t version = INVALID_STMT_VERSION) : _version(version),
                                                            _frequency(0) {}

    virtual ~SSAStmt() = default;

    virtual void print(std::ostream_iterator<char>& out) const noexcept = 0;

    virtual std::uint64_t canonicalize() const noexcept = 0;

    virtual int type_id() const noexcept = 0;

    virtual std::vector<SSAStmt*> operands() noexcept { return {}; }

    virtual void replace_operand(size_t idx, SSAStmt* repl) noexcept {}

    std::uint64_t get_version() const noexcept { return this->_version; }

    void set_version(uint64_t version) noexcept { this->_version = version; }

    void increment_frequency() noexcept { this->_frequency++; }

    std::uint64_t get_frequency() const noexcept { return this->_frequency; }
};

class MATHEXPR_API SSAStmtVariable : public SSAStmt
{
    std::string_view _name;

public:
    SSAStmtVariable(std::string_view name,
                    std::uint64_t version = INVALID_STMT_VERSION) : SSAStmt(version),
                                                                    _name(name) {}

    virtual ~SSAStmtVariable() override {}

    virtual void print(std::ostream_iterator<char>& out) const noexcept override;

    virtual std::uint64_t canonicalize() const noexcept override;

    static constexpr int static_type_id() { return SSAStmtTypeId_Variable; }

    virtual int type_id() const noexcept override { return this->static_type_id(); }

    std::string_view get_name() const noexcept { return this->_name; }
};

class MATHEXPR_API SSAStmtLiteral : public SSAStmt
{
    std::string_view _name;

public:
    SSAStmtLiteral(std::string_view name,
                   std::uint64_t version = INVALID_STMT_VERSION) : SSAStmt(version),
                                                                   _name(name) {}

    virtual ~SSAStmtLiteral() override {}

    virtual void print(std::ostream_iterator<char>& out) const noexcept override;

    virtual std::uint64_t canonicalize() const noexcept override;

    static constexpr int static_type_id() { return SSAStmtTypeId_Literal; }

    virtual int type_id() const noexcept override { return this->static_type_id(); }

    std::string_view get_name() const noexcept { return this->_name; }
};

class MATHEXPR_API SSAStmtUnOp : public SSAStmt
{
    SSAStmt* _operand;

    UnaryOpType _op;

public:
    SSAStmtUnOp(SSAStmt* operand,
                UnaryOpType op,
                std::uint64_t version = INVALID_STMT_VERSION) : SSAStmt(version),
                                                                _operand(operand),
                                                                _op(op) {}

    virtual ~SSAStmtUnOp() override {}

    virtual void print(std::ostream_iterator<char>& out) const noexcept override;

    virtual std::uint64_t canonicalize() const noexcept override;

    static constexpr int static_type_id() { return SSAStmtTypeId_UnOp; }

    virtual int type_id() const noexcept override { return this->static_type_id(); }

    virtual std::vector<SSAStmt*> operands() noexcept override { return { this->_operand }; }

    virtual void replace_operand(size_t idx, SSAStmt* repl) noexcept override { MATHEXPR_ASSERT(idx == 0, "idx cannot be greater than 0"); this->_operand = repl; }

    SSAStmt*& get_operand() noexcept { return this->_operand; }

    const SSAStmt* get_operand() const noexcept { return this->_operand; }

    void set_operand(SSAStmt*& operand) noexcept { this->_operand = operand; }

    UnaryOpType get_op() const noexcept { return this->_op; }
};

class MATHEXPR_API SSAStmtBinOp : public SSAStmt
{
    SSAStmt* _left;
    SSAStmt* _right;

    BinaryOpType _op;

public:
    SSAStmtBinOp(SSAStmt* left,
                 SSAStmt* right,
                 BinaryOpType op,
                 std::uint64_t version = INVALID_STMT_VERSION) : SSAStmt(version),
                                                                 _left(left),
                                                                 _right(right),
                                                                 _op(op) {}

    virtual ~SSAStmtBinOp() override {}

    virtual void print(std::ostream_iterator<char>& out) const noexcept override;

    virtual uint64_t canonicalize() const noexcept override;

    static constexpr int static_type_id() { return SSAStmtTypeId_BinOp; }

    virtual int type_id() const noexcept override { return this->static_type_id(); }

    virtual std::vector<SSAStmt*> operands() noexcept override { return { this->_left, this->_right }; }

    virtual void replace_operand(std::size_t idx, SSAStmt* repl) noexcept override
    {
        MATHEXPR_ASSERT(idx <= 1, "idx cannot be greater than 1");

        switch(idx)
        {
            case 0:
                this->_left = repl;
                break;
            case 1:
                this->_right = repl;
                break;
            default:
                break;
        }
    }

    SSAStmt*& get_left() noexcept { return this->_left; }

    const SSAStmt* get_left() const noexcept { return this->_left; }

    void set_left(SSAStmt*& left) noexcept { this->_left = left; }

    SSAStmt*& get_right() noexcept { return this->_right; }

    const SSAStmt* get_right() const noexcept { return this->_right; }

    void set_right(SSAStmt*& right) noexcept { this->_right = right; }

    void swap_operands() noexcept { std::swap(this->_left, this->_right); }

    BinaryOpType get_op() const noexcept { return this->_op; }
};

class MATHEXPR_API SSAStmtFunctionOp : public SSAStmt
{
    std::vector<SSAStmt*> _arguments;

    std::string_view _name;

public:
    SSAStmtFunctionOp(std::string_view name,
                      std::vector<SSAStmt*> arguments,
                      std::uint64_t version = INVALID_STMT_VERSION) : SSAStmt(version),
                                                                      _name(name),
                                                                      _arguments(std::move(arguments)) {}

    virtual ~SSAStmtFunctionOp() override {}

    virtual void print(std::ostream_iterator<char>& out) const noexcept override;

    virtual uint64_t canonicalize() const noexcept override;

    static constexpr int static_type_id() { return SSAStmtTypeId_FuncOp; }

    virtual int type_id() const noexcept override { return this->static_type_id(); }

    virtual std::vector<SSAStmt*> operands() noexcept override { return this->_arguments; }

    virtual void replace_operand(std::size_t idx, SSAStmt* repl) noexcept override
    {
        MATHEXPR_ASSERT(idx <= this->_arguments.size(), "idx cannot be greater than number of arguments");

        if(idx >= this->_arguments.size())
            return;

        this->_arguments[idx] = repl;
    }

    std::string_view get_name() const noexcept { return this->_name; }

    std::vector<SSAStmt*>& get_arguments() noexcept { return this->_arguments; }

    const std::vector<SSAStmt*>& get_arguments() const noexcept { return this->_arguments; }
};

template<typename T>
const T* statement_const_cast(const SSAStmt* stmt) noexcept
{
    if(stmt != nullptr && stmt->type_id() == T::static_type_id())
        return static_cast<const T*>(stmt);

    return nullptr;
}

template<typename T>
T* statement_cast(SSAStmt* stmt) noexcept
{
    if(stmt != nullptr && stmt->type_id() == T::static_type_id())
        return static_cast<T*>(stmt);

    return nullptr;
}

MATHEXPR_API bool ssa_statement_needs_register(const SSAStmt* stmt) noexcept;

class MATHEXPR_API SSA
{
    std::vector<SSAStmt*> _statements;

    SlabAllocator& _slab;

    std::uint64_t get_statement_number() const noexcept { return this->_statements.size(); }

public:
    SSA(SlabAllocator& slab) : _slab(slab) {}

    bool calculate_live_ranges() noexcept;

    void print() const noexcept;

    bool build_from_ast(const AST& ast) noexcept;

    const std::vector<SSAStmt*>& get_statements() const noexcept { return this->_statements; }

    std::vector<SSAStmt*>& get_statements() noexcept { return this->_statements; }
};

MATHEXPR_NAMESPACE_END

#endif /* !defined(__MATHEXPR_SSA) */
