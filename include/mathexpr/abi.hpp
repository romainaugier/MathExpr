// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHEXPR_ABI)
#define __MATHEXPR_ABI

#include "mathexpr/platform.hpp"

#include <span>
#include <string_view>

/*
    ABIs

    https://www.thejat.in/learn/system-v-amd64-calling-convention
    https://learn.microsoft.com/en-us/cpp/build/x64-software-conventions?view=msvc-170#x64-register-usage
    https://developer.apple.com/documentation/xcode/writing-arm64-code-for-apple-platforms
    https://github.com/ARM-software/abi-aa/blob/main/aapcs64/aapcs64.rst#machine-registers
    https://docs.nvidia.com/cuda/ptx-writers-guide-to-interoperability/index.html
*/

MATHEXPR_NAMESPACE_BEGIN

enum PlatformABIID : std::uint32_t
{
    PlatformABIID_WindowsX64,
    PlatformABIID_LinuxX64,
    PlatformABIID_AppleARM64,
    PlatformABIID_NvPTX,
};

class MATHEXPR_API PlatformABI
{
public:
    virtual ~PlatformABI() = default;

    virtual std::string_view get_as_string() const noexcept = 0;

    virtual std::uint32_t get_id() const noexcept = 0;

    virtual ISA get_target_isa() const noexcept = 0;

    virtual bool can_fold_memory_operand() const noexcept { return false; }

    virtual bool has_two_address_form() const noexcept { return false; }

    virtual bool has_virtual_registers() const noexcept { return false; }

    virtual std::uint64_t stack_alignment() const noexcept { return 16; }

    /* base ptr for the variables values, passed as the first parameter */
    virtual RegisterId get_variable_base_ptr() const noexcept = 0;

    /* base ptr for the literals values, passed as the second parameter */
    virtual RegisterId get_literal_base_ptr() const noexcept = 0;

    /* scratch register used to hold the target address of an indirect call */
    virtual RegisterId get_function_call_ptr() const noexcept = 0;

    /*
        Register allocation priority lists. Caller-saved registers come first,
        callee-saved registers are appended at the end so the register allocator
        only uses them when spilling would otherwise be required.
    */
    virtual std::span<const RegisterId> get_gp_allocatable_registers() const noexcept = 0;
    virtual std::span<const RegisterId> get_fp_allocatable_registers() const noexcept = 0;

    /* Registers clobbered by a call (subset of the allocatable lists) */
    virtual std::span<const RegisterId> get_caller_saved_gp_registers() const noexcept = 0;
    virtual std::span<const RegisterId> get_caller_saved_fp_registers() const noexcept = 0;

    /* Registers the prologue must save if the allocator ends up using them */
    virtual std::span<const RegisterId> get_callee_saved_gp_registers() const noexcept = 0;
    virtual std::span<const RegisterId> get_callee_saved_fp_registers() const noexcept = 0;

    /* Registers used to return a value from a call */
    virtual RegisterId get_call_return_value_gp_register() const noexcept = 0;
    virtual RegisterId get_call_return_value_fp_register() const noexcept = 0;

    /* Registers used to pass arguments to a call, in argument order */
    virtual std::span<const RegisterId> get_call_args_gp_registers() const noexcept = 0;
    virtual std::span<const RegisterId> get_call_args_fp_registers() const noexcept = 0;

    /* Outgoing-args shadow space the callee may rely on (Win64: 32 bytes) */
    virtual std::uint64_t get_call_shadow_space_size() const noexcept { return 0; }

    /* Bytes below SP a leaf function may use without adjusting SP (SysV/AAPCS64 red zone) */
    virtual std::uint64_t get_stack_red_zone_size() const noexcept { return 0; }

    /* Offset from the frame base where spill slots start */
    virtual std::uint64_t get_stack_base_offset() const noexcept { return 0; }
};

const PlatformABI* get_platform_abi(ISA isa = get_current_isa(),
                                    Platform platform = get_current_platform()) noexcept;

class MATHEXPR_API WindowsX64ABI : public PlatformABI
{
public:
    virtual ~WindowsX64ABI() = default;

    virtual std::string_view get_as_string() const noexcept override;

    virtual std::uint32_t get_id() const noexcept override;

    virtual ISA get_target_isa() const noexcept override;

    virtual bool can_fold_memory_operand() const noexcept override;

    virtual bool has_two_address_form() const noexcept override;

    virtual RegisterId get_variable_base_ptr() const noexcept override;

    virtual RegisterId get_literal_base_ptr() const noexcept override;

    virtual RegisterId get_function_call_ptr() const noexcept override;

    virtual std::span<const RegisterId> get_gp_allocatable_registers() const noexcept override;
    virtual std::span<const RegisterId> get_fp_allocatable_registers() const noexcept override;

    virtual std::span<const RegisterId> get_caller_saved_gp_registers() const noexcept override;
    virtual std::span<const RegisterId> get_caller_saved_fp_registers() const noexcept override;

    virtual std::span<const RegisterId> get_callee_saved_gp_registers() const noexcept override;
    virtual std::span<const RegisterId> get_callee_saved_fp_registers() const noexcept override;

    virtual RegisterId get_call_return_value_gp_register() const noexcept override;
    virtual RegisterId get_call_return_value_fp_register() const noexcept override;

    virtual std::span<const RegisterId> get_call_args_gp_registers() const noexcept override;
    virtual std::span<const RegisterId> get_call_args_fp_registers() const noexcept override;

    virtual std::uint64_t get_call_shadow_space_size() const noexcept override;

    virtual std::uint64_t get_stack_base_offset() const noexcept override;
};

/* SysV AMD64 */
class MATHEXPR_API LinuxX64ABI : public PlatformABI
{
public:
    virtual ~LinuxX64ABI() = default;

    virtual std::string_view get_as_string() const noexcept override;

    virtual std::uint32_t get_id() const noexcept override;

    virtual ISA get_target_isa() const noexcept override;

    virtual bool can_fold_memory_operand() const noexcept override;

    virtual bool has_two_address_form() const noexcept override;

    virtual RegisterId get_variable_base_ptr() const noexcept override;

    virtual RegisterId get_literal_base_ptr() const noexcept override;

    virtual RegisterId get_function_call_ptr() const noexcept override;

    virtual std::span<const RegisterId> get_gp_allocatable_registers() const noexcept override;
    virtual std::span<const RegisterId> get_fp_allocatable_registers() const noexcept override;

    virtual std::span<const RegisterId> get_caller_saved_gp_registers() const noexcept override;
    virtual std::span<const RegisterId> get_caller_saved_fp_registers() const noexcept override;

    virtual std::span<const RegisterId> get_callee_saved_gp_registers() const noexcept override;
    virtual std::span<const RegisterId> get_callee_saved_fp_registers() const noexcept override;

    virtual RegisterId get_call_return_value_gp_register() const noexcept override;
    virtual RegisterId get_call_return_value_fp_register() const noexcept override;

    virtual std::span<const RegisterId> get_call_args_gp_registers() const noexcept override;
    virtual std::span<const RegisterId> get_call_args_fp_registers() const noexcept override;

    virtual std::uint64_t get_stack_red_zone_size() const noexcept override;

    virtual std::uint64_t get_stack_base_offset() const noexcept override;
};

class MATHEXPR_API AppleARM64ABI : public PlatformABI
{
public:
    virtual ~AppleARM64ABI() = default;

    virtual std::string_view get_as_string() const noexcept override;

    virtual std::uint32_t get_id() const noexcept override;

    virtual ISA get_target_isa() const noexcept override;

    virtual RegisterId get_variable_base_ptr() const noexcept override;

    virtual RegisterId get_literal_base_ptr() const noexcept override;

    virtual RegisterId get_function_call_ptr() const noexcept override;

    virtual std::span<const RegisterId> get_gp_allocatable_registers() const noexcept override;
    virtual std::span<const RegisterId> get_fp_allocatable_registers() const noexcept override;

    virtual std::span<const RegisterId> get_caller_saved_gp_registers() const noexcept override;
    virtual std::span<const RegisterId> get_caller_saved_fp_registers() const noexcept override;

    virtual std::span<const RegisterId> get_callee_saved_gp_registers() const noexcept override;
    virtual std::span<const RegisterId> get_callee_saved_fp_registers() const noexcept override;

    virtual RegisterId get_call_return_value_gp_register() const noexcept override;
    virtual RegisterId get_call_return_value_fp_register() const noexcept override;

    virtual std::span<const RegisterId> get_call_args_gp_registers() const noexcept override;
    virtual std::span<const RegisterId> get_call_args_fp_registers() const noexcept override;

    virtual std::uint64_t get_stack_red_zone_size() const noexcept override;

    virtual std::uint64_t get_stack_base_offset() const noexcept override;
};

class MATHEXPR_API NvPTXABI : public PlatformABI
{
public:
    virtual ~NvPTXABI() = default;

    virtual std::string_view get_as_string() const noexcept override;

    virtual std::uint32_t get_id() const noexcept override;

    virtual ISA get_target_isa() const noexcept override;

    virtual bool has_virtual_registers() const noexcept override;

    virtual RegisterId get_variable_base_ptr() const noexcept override;

    virtual RegisterId get_literal_base_ptr() const noexcept override;

    virtual RegisterId get_function_call_ptr() const noexcept override;

    virtual std::span<const RegisterId> get_gp_allocatable_registers() const noexcept override;
    virtual std::span<const RegisterId> get_fp_allocatable_registers() const noexcept override;

    virtual std::span<const RegisterId> get_caller_saved_gp_registers() const noexcept override;
    virtual std::span<const RegisterId> get_caller_saved_fp_registers() const noexcept override;

    virtual std::span<const RegisterId> get_callee_saved_gp_registers() const noexcept override;
    virtual std::span<const RegisterId> get_callee_saved_fp_registers() const noexcept override;

    virtual RegisterId get_call_return_value_gp_register() const noexcept override;
    virtual RegisterId get_call_return_value_fp_register() const noexcept override;

    virtual std::span<const RegisterId> get_call_args_gp_registers() const noexcept override;
    virtual std::span<const RegisterId> get_call_args_fp_registers() const noexcept override;
};

MATHEXPR_NAMESPACE_END

#endif /* !defined(__MATHEXPR_ABI) */