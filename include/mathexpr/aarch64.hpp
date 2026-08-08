// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHEXPR_AARCH64)
#define __MATHEXPR_AARCH64

#include "mathexpr/codegen.hpp"

#define AARCH64_NAMESPACE_BEGIN namespace aarch64 {
#define AARCH64_NAMESPACE_END }

MATHEXPR_NAMESPACE_BEGIN

AARCH64_NAMESPACE_BEGIN

class MATHEXPR_API InstrMov : public Instr
{
    MemLoc _mem_loc_from;
    MemLoc _mem_loc_to;

public:
    InstrMov(MemLoc& from, MemLoc& to) : _mem_loc_from(from),
                                         _mem_loc_to(to) {}

    virtual void as_string(std::string& out) const noexcept override;
    virtual void as_bytecode(ByteCode& out) const noexcept override;
};

class MATHEXPR_API InstrPrologue : public Instr
{
    uint64_t _stack_size;

public:
    InstrPrologue(uint64_t stack_size) : _stack_size(stack_size) {}

    virtual void as_string(std::string& out) const noexcept override;
    virtual void as_bytecode(ByteCode& out) const noexcept override;
};

class MATHEXPR_API InstrEpilogue : public Instr
{
    uint64_t _stack_size;

public:
    InstrEpilogue(uint64_t stack_size) : _stack_size(stack_size) {}

    virtual void as_string(std::string& out) const noexcept override;
    virtual void as_bytecode(ByteCode& out) const noexcept override;
};

/* Unary ops instructions */

class MATHEXPR_API InstrNeg : public Instr
{
    MemLoc _operand;

public:
    InstrNeg(MemLoc& operand) : _operand(operand) {}

    virtual void as_string(std::string& out) const noexcept override;
    virtual void as_bytecode(ByteCode& out) const noexcept override;
};

/* Binary ops instructions */

class MATHEXPR_API InstrAdd : public Instr
{
    MemLoc _left;
    MemLoc _right;

public:
    InstrAdd(MemLoc& left, MemLoc& right) : _left(left),
                                            _right(right) {}

    virtual void as_string(std::string& out) const noexcept override;
    virtual void as_bytecode(ByteCode& out) const noexcept override;
};

class MATHEXPR_API InstrSub : public Instr
{
    MemLoc _left;
    MemLoc _right;

public:
    InstrSub(MemLoc& left, MemLoc& right) : _left(left),
                                            _right(right) {}

    virtual void as_string(std::string& out) const noexcept override;
    virtual void as_bytecode(ByteCode& out) const noexcept override;
};

class MATHEXPR_API InstrMul : public Instr
{
    MemLoc _left;
    MemLoc _right;

public:
    InstrMul(MemLoc& left, MemLoc& right) : _left(left),
                                            _right(right) {}

    virtual void as_string(std::string& out) const noexcept override;
    virtual void as_bytecode(ByteCode& out) const noexcept override;
};

class MATHEXPR_API InstrDiv : public Instr
{
    MemLoc _left;
    MemLoc _right;

public:
    InstrDiv(MemLoc& left, MemLoc& right) : _left(left),
                                            _right(right) {}

    virtual void as_string(std::string& out) const noexcept override;
    virtual void as_bytecode(ByteCode& out) const noexcept override;
};

/* Func ops instructions */

class MATHEXPR_API InstrCall : public Instr
{
    std::string_view _call_name;

public:
    InstrCall(std::string_view call_name) : _call_name(call_name) {}

    virtual void as_string(std::string& out) const noexcept override;
    virtual void as_bytecode(ByteCode& out) const noexcept override;

    virtual bool needs_linking() const noexcept override { return true; }

    virtual RelocInfo get_link_info(std::size_t bytecode_start) const noexcept override;
};

/* Terminator instructions */

class MATHEXPR_API InstrRet : public Instr
{
public:

    virtual void as_string(std::string& out) const noexcept override;
    virtual void as_bytecode(ByteCode& out) const noexcept override;
};

AARCH64_NAMESPACE_END

class MATHEXPR_API AARCH64_CodeGenerator : public TargetCodeGenerator
{
public:
    AARCH64_CodeGenerator(PlatformABIPtr platform_abi) : TargetCodeGenerator(platform_abi) {}

    virtual ~AARCH64_CodeGenerator() override = default;

    virtual bool is_valid() const noexcept override { return get_current_isa() == ISA_aarch64; }

    virtual InstrPtr create_mov(MemLoc& from, MemLoc& to) override;
    virtual InstrPtr create_prologue(uint64_t stack_size) override;
    virtual InstrPtr create_epilogue(uint64_t stack_size) override;
    virtual InstrPtr create_neg(MemLoc& operand) override;
    virtual InstrPtr create_add(MemLoc& left, MemLoc& right) override;
    virtual InstrPtr create_sub(MemLoc& left, MemLoc& right) override;
    virtual InstrPtr create_mul(MemLoc& left, MemLoc& right) override;
    virtual InstrPtr create_div(MemLoc& left, MemLoc& right) override;
    virtual InstrPtr create_call(std::string_view call_name) override;
    virtual InstrPtr create_ret() override;

    virtual void optimize_instr_sequence(std::vector<InstrPtr>& instructions) noexcept override;
};

MATHEXPR_NAMESPACE_END

#endif /* !defined(__MATHEXPR_AARCH64) */
