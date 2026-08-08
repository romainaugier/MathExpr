// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#include "mathexpr/abi.hpp"

#include <array>

MATHEXPR_NAMESPACE_BEGIN

const PlatformABI* get_platform_abi(ISA isa, Platform platform) noexcept
{
    switch(isa)
    {
        case ISA::x86_64:
        {
            switch(platform)
            {
                case Platform::Windows:
                {
                    static const WindowsX64ABI abi;
                    return std::addressof(abi);
                }

                case Platform::Linux:
                {
                    static const LinuxX64ABI abi;
                    return std::addressof(abi);
                }
            }
        }

        case ISA::aarch64:
        {
            switch(platform)
            {
                case Platform::Apple:
                {
                    static const AppleARM64ABI abi;
                    return std::addressof(abi);
                }
            }
        }
    }

    return nullptr;
}

/*
    Windows x64
    GP volatile: RAX, RCX, RDX, R8-R11 | non-volatile: RBX, RBP, RSI, RDI, R12-R15
    FP volatile: XMM0-XMM5             | non-volatile: XMM6-XMM15
    RCX/RDX are reserved for the variable/literal base pointers, RAX for the
    indirect call target, so they are excluded from the allocatable list.
*/
static constexpr std::array<RegisterId, 11> winx64_gp_allocatable = {
    GpRegisters_x86_64_R8,  GpRegisters_x86_64_R9,  GpRegisters_x86_64_R10,
    GpRegisters_x86_64_R11,
    /* callee-saved below, used only to avoid spilling */
    GpRegisters_x86_64_RBX, GpRegisters_x86_64_RSI, GpRegisters_x86_64_RDI,
    GpRegisters_x86_64_R12, GpRegisters_x86_64_R13, GpRegisters_x86_64_R14,
    GpRegisters_x86_64_R15,
};

/*
    FP priority: XMM4/XMM5 first (volatile, never used for args/retval),
    then XMM0-XMM3 (volatile but clobbered by calls / used for retval),
    then callee-saved XMM6-XMM15.
*/
static constexpr std::array<RegisterId, 16> winx64_fp_allocatable = {
    FpRegisters_x86_64_Xmm4,  FpRegisters_x86_64_Xmm5,
    FpRegisters_x86_64_Xmm0,  FpRegisters_x86_64_Xmm1,
    FpRegisters_x86_64_Xmm2,  FpRegisters_x86_64_Xmm3,
    /* callee-saved below */
    FpRegisters_x86_64_Xmm6,  FpRegisters_x86_64_Xmm7,
    FpRegisters_x86_64_Xmm8,  FpRegisters_x86_64_Xmm9,
    FpRegisters_x86_64_Xmm10, FpRegisters_x86_64_Xmm11,
    FpRegisters_x86_64_Xmm12, FpRegisters_x86_64_Xmm13,
    FpRegisters_x86_64_Xmm14, FpRegisters_x86_64_Xmm15,
};

static constexpr std::array<RegisterId, 4> winx64_gp_caller_saved = {
    GpRegisters_x86_64_R8, GpRegisters_x86_64_R9,
    GpRegisters_x86_64_R10, GpRegisters_x86_64_R11,
};

static constexpr std::array<RegisterId, 6> winx64_fp_caller_saved = {
    FpRegisters_x86_64_Xmm0, FpRegisters_x86_64_Xmm1, FpRegisters_x86_64_Xmm2,
    FpRegisters_x86_64_Xmm3, FpRegisters_x86_64_Xmm4, FpRegisters_x86_64_Xmm5,
};

static constexpr std::array<RegisterId, 7> winx64_gp_callee_saved = {
    GpRegisters_x86_64_RBX, GpRegisters_x86_64_RSI, GpRegisters_x86_64_RDI,
    GpRegisters_x86_64_R12, GpRegisters_x86_64_R13, GpRegisters_x86_64_R14,
    GpRegisters_x86_64_R15,
};

static constexpr std::array<RegisterId, 10> winx64_fp_callee_saved = {
    FpRegisters_x86_64_Xmm6,  FpRegisters_x86_64_Xmm7,
    FpRegisters_x86_64_Xmm8,  FpRegisters_x86_64_Xmm9,
    FpRegisters_x86_64_Xmm10, FpRegisters_x86_64_Xmm11,
    FpRegisters_x86_64_Xmm12, FpRegisters_x86_64_Xmm13,
    FpRegisters_x86_64_Xmm14, FpRegisters_x86_64_Xmm15,
};

static constexpr std::array<RegisterId, 4> winx64_call_args_gp = {
    GpRegisters_x86_64_RCX, GpRegisters_x86_64_RDX,
    GpRegisters_x86_64_R8,  GpRegisters_x86_64_R9,
};

static constexpr std::array<RegisterId, 4> winx64_call_args_fp = {
    FpRegisters_x86_64_Xmm0, FpRegisters_x86_64_Xmm1,
    FpRegisters_x86_64_Xmm2, FpRegisters_x86_64_Xmm3,
};

/*
    SysV AMD64
    GP volatile: RAX, RCX, RDX, RSI, RDI, R8-R11 | non-volatile: RBX, RBP, R12-R15
    FP: all XMM0-XMM15 volatile
    RDI/RSI reserved for the variable/literal base pointers, RAX for the
    indirect call target.
*/
static constexpr std::array<RegisterId, 11> linuxx64_gp_allocatable = {
    GpRegisters_x86_64_R10, GpRegisters_x86_64_R11,
    GpRegisters_x86_64_RCX, GpRegisters_x86_64_RDX,
    GpRegisters_x86_64_RSI, GpRegisters_x86_64_RDI,
    GpRegisters_x86_64_R8,  GpRegisters_x86_64_R9,
    /* callee-saved below */
    GpRegisters_x86_64_RBX, GpRegisters_x86_64_R12, GpRegisters_x86_64_R13,
    /* note: R14/R15 could be added here if we don't need them elsewhere */
};

static constexpr std::array<RegisterId, 16> linuxx64_fp_allocatable = {
    FpRegisters_x86_64_Xmm8,  FpRegisters_x86_64_Xmm9,
    FpRegisters_x86_64_Xmm10, FpRegisters_x86_64_Xmm11,
    FpRegisters_x86_64_Xmm12, FpRegisters_x86_64_Xmm13,
    FpRegisters_x86_64_Xmm14, FpRegisters_x86_64_Xmm15,
    FpRegisters_x86_64_Xmm4,  FpRegisters_x86_64_Xmm5,
    FpRegisters_x86_64_Xmm6,  FpRegisters_x86_64_Xmm7,
    FpRegisters_x86_64_Xmm0,  FpRegisters_x86_64_Xmm1,
    FpRegisters_x86_64_Xmm2,  FpRegisters_x86_64_Xmm3,
};

static constexpr std::array<RegisterId, 8> linuxx64_gp_caller_saved = {
    GpRegisters_x86_64_RCX, GpRegisters_x86_64_RDX,
    GpRegisters_x86_64_RSI, GpRegisters_x86_64_RDI,
    GpRegisters_x86_64_R8,  GpRegisters_x86_64_R9,
    GpRegisters_x86_64_R10, GpRegisters_x86_64_R11,
};

static constexpr std::array<RegisterId, 16> linuxx64_fp_caller_saved = {
    FpRegisters_x86_64_Xmm0,  FpRegisters_x86_64_Xmm1,
    FpRegisters_x86_64_Xmm2,  FpRegisters_x86_64_Xmm3,
    FpRegisters_x86_64_Xmm4,  FpRegisters_x86_64_Xmm5,
    FpRegisters_x86_64_Xmm6,  FpRegisters_x86_64_Xmm7,
    FpRegisters_x86_64_Xmm8,  FpRegisters_x86_64_Xmm9,
    FpRegisters_x86_64_Xmm10, FpRegisters_x86_64_Xmm11,
    FpRegisters_x86_64_Xmm12, FpRegisters_x86_64_Xmm13,
    FpRegisters_x86_64_Xmm14, FpRegisters_x86_64_Xmm15,
};

static constexpr std::array<RegisterId, 3> linuxx64_gp_callee_saved = {
    GpRegisters_x86_64_RBX, GpRegisters_x86_64_R12, GpRegisters_x86_64_R13,
};

/* arg order: RDI, RSI, RDX, RCX, R8, R9 */
static constexpr std::array<RegisterId, 6> linuxx64_call_args_gp = {
    GpRegisters_x86_64_RDI, GpRegisters_x86_64_RSI,
    GpRegisters_x86_64_RDX, GpRegisters_x86_64_RCX,
    GpRegisters_x86_64_R8,  GpRegisters_x86_64_R9,
};

static constexpr std::array<RegisterId, 8> linuxx64_call_args_fp = {
    FpRegisters_x86_64_Xmm0, FpRegisters_x86_64_Xmm1,
    FpRegisters_x86_64_Xmm2, FpRegisters_x86_64_Xmm3,
    FpRegisters_x86_64_Xmm4, FpRegisters_x86_64_Xmm5,
    FpRegisters_x86_64_Xmm6, FpRegisters_x86_64_Xmm7,
};

/*
    Apple ARM64 (AAPCS64 with Darwin deviations)
    X0-X18 volatile (X18 reserved by Darwin, excluded), X19-X29 non-volatile
    V0-V7 argument/result, V8-V15 callee-saved (lower 64 bits only),
    V16-V31 volatile
    X0/X1 reserved for the variable/literal base pointers, X9 for the
    indirect call target.
*/
static constexpr std::array<RegisterId, 25> arm64_gp_allocatable = {
    GpRegisters_aarch64_X2,  GpRegisters_aarch64_X3,
    GpRegisters_aarch64_X4,  GpRegisters_aarch64_X5,
    GpRegisters_aarch64_X6,  GpRegisters_aarch64_X7,
    GpRegisters_aarch64_X8,  GpRegisters_aarch64_X10,
    GpRegisters_aarch64_X11, GpRegisters_aarch64_X12,
    GpRegisters_aarch64_X13, GpRegisters_aarch64_X14,
    GpRegisters_aarch64_X15, GpRegisters_aarch64_X16,
    GpRegisters_aarch64_X17,
    /* callee-saved below */
    GpRegisters_aarch64_X19, GpRegisters_aarch64_X20,
    GpRegisters_aarch64_X21, GpRegisters_aarch64_X22,
    GpRegisters_aarch64_X23, GpRegisters_aarch64_X24,
    GpRegisters_aarch64_X25, GpRegisters_aarch64_X26,
    GpRegisters_aarch64_X27, GpRegisters_aarch64_X28,
};

static constexpr std::array<RegisterId, 32> arm64_fp_allocatable = {
    /* volatile temporaries first */
    FpRegisters_aarch64_V16, FpRegisters_aarch64_V17,
    FpRegisters_aarch64_V18, FpRegisters_aarch64_V19,
    FpRegisters_aarch64_V20, FpRegisters_aarch64_V21,
    FpRegisters_aarch64_V22, FpRegisters_aarch64_V23,
    FpRegisters_aarch64_V24, FpRegisters_aarch64_V25,
    FpRegisters_aarch64_V26, FpRegisters_aarch64_V27,
    FpRegisters_aarch64_V28, FpRegisters_aarch64_V29,
    FpRegisters_aarch64_V30, FpRegisters_aarch64_V31,
    /* argument/result registers (call-clobbered) */
    FpRegisters_aarch64_V2,  FpRegisters_aarch64_V3,
    FpRegisters_aarch64_V4,  FpRegisters_aarch64_V5,
    FpRegisters_aarch64_V6,  FpRegisters_aarch64_V7,
    FpRegisters_aarch64_V0,  FpRegisters_aarch64_V1,
    /* callee-saved below (D8-D15, i.e. low 64 bits of V8-V15) */
    FpRegisters_aarch64_V8,  FpRegisters_aarch64_V9,
    FpRegisters_aarch64_V10, FpRegisters_aarch64_V11,
    FpRegisters_aarch64_V12, FpRegisters_aarch64_V13,
    FpRegisters_aarch64_V14, FpRegisters_aarch64_V15,
};

static constexpr std::array<RegisterId, 15> arm64_gp_caller_saved = {
    GpRegisters_aarch64_X2,  GpRegisters_aarch64_X3,
    GpRegisters_aarch64_X4,  GpRegisters_aarch64_X5,
    GpRegisters_aarch64_X6,  GpRegisters_aarch64_X7,
    GpRegisters_aarch64_X8,  GpRegisters_aarch64_X10,
    GpRegisters_aarch64_X11, GpRegisters_aarch64_X12,
    GpRegisters_aarch64_X13, GpRegisters_aarch64_X14,
    GpRegisters_aarch64_X15, GpRegisters_aarch64_X16,
    GpRegisters_aarch64_X17,
};

static constexpr std::array<RegisterId, 24> arm64_fp_caller_saved = {
    FpRegisters_aarch64_V0,  FpRegisters_aarch64_V1,
    FpRegisters_aarch64_V2,  FpRegisters_aarch64_V3,
    FpRegisters_aarch64_V4,  FpRegisters_aarch64_V5,
    FpRegisters_aarch64_V6,  FpRegisters_aarch64_V7,
    FpRegisters_aarch64_V16, FpRegisters_aarch64_V17,
    FpRegisters_aarch64_V18, FpRegisters_aarch64_V19,
    FpRegisters_aarch64_V20, FpRegisters_aarch64_V21,
    FpRegisters_aarch64_V22, FpRegisters_aarch64_V23,
    FpRegisters_aarch64_V24, FpRegisters_aarch64_V25,
    FpRegisters_aarch64_V26, FpRegisters_aarch64_V27,
    FpRegisters_aarch64_V28, FpRegisters_aarch64_V29,
    FpRegisters_aarch64_V30, FpRegisters_aarch64_V31,
};

static constexpr std::array<RegisterId, 10> arm64_gp_callee_saved = {
    GpRegisters_aarch64_X19, GpRegisters_aarch64_X20,
    GpRegisters_aarch64_X21, GpRegisters_aarch64_X22,
    GpRegisters_aarch64_X23, GpRegisters_aarch64_X24,
    GpRegisters_aarch64_X25, GpRegisters_aarch64_X26,
    GpRegisters_aarch64_X27, GpRegisters_aarch64_X28,
};

static constexpr std::array<RegisterId, 8> arm64_fp_callee_saved = {
    FpRegisters_aarch64_V8,  FpRegisters_aarch64_V9,
    FpRegisters_aarch64_V10, FpRegisters_aarch64_V11,
    FpRegisters_aarch64_V12, FpRegisters_aarch64_V13,
    FpRegisters_aarch64_V14, FpRegisters_aarch64_V15,
};

static constexpr std::array<RegisterId, 8> arm64_call_args_gp = {
    GpRegisters_aarch64_X0, GpRegisters_aarch64_X1,
    GpRegisters_aarch64_X2, GpRegisters_aarch64_X3,
    GpRegisters_aarch64_X4, GpRegisters_aarch64_X5,
    GpRegisters_aarch64_X6, GpRegisters_aarch64_X7,
};

static constexpr std::array<RegisterId, 8> arm64_call_args_fp = {
    FpRegisters_aarch64_V0, FpRegisters_aarch64_V1,
    FpRegisters_aarch64_V2, FpRegisters_aarch64_V3,
    FpRegisters_aarch64_V4, FpRegisters_aarch64_V5,
    FpRegisters_aarch64_V6, FpRegisters_aarch64_V7,
};

/* WindowsX64ABI */

std::string_view WindowsX64ABI::get_as_string() const noexcept
{
    return "Windows x64";
}

std::uint32_t WindowsX64ABI::get_id() const noexcept
{
    return PlatformABIID_WindowsX64;
}

ISA WindowsX64ABI::get_target_isa() const noexcept
{
    return ISA::x86_64;
}

bool WindowsX64ABI::can_fold_memory_operand() const noexcept
{
    return true;
}

bool WindowsX64ABI::has_two_address_form() const noexcept
{
    return true;
}

RegisterId WindowsX64ABI::get_variable_base_ptr() const noexcept
{
    return GpRegisters_x86_64_RCX;
}

RegisterId WindowsX64ABI::get_literal_base_ptr() const noexcept
{
    return GpRegisters_x86_64_RDX;
}

RegisterId WindowsX64ABI::get_function_call_ptr() const noexcept
{
    return GpRegisters_x86_64_RAX;
}

std::span<const RegisterId> WindowsX64ABI::get_gp_allocatable_registers() const noexcept
{
    return winx64_gp_allocatable;
}

std::span<const RegisterId> WindowsX64ABI::get_fp_allocatable_registers() const noexcept
{
    return winx64_fp_allocatable;
}

std::span<const RegisterId> WindowsX64ABI::get_caller_saved_gp_registers() const noexcept
{
    return winx64_gp_caller_saved;
}

std::span<const RegisterId> WindowsX64ABI::get_caller_saved_fp_registers() const noexcept
{
    return winx64_fp_caller_saved;
}

std::span<const RegisterId> WindowsX64ABI::get_callee_saved_gp_registers() const noexcept
{
    return winx64_gp_callee_saved;
}

std::span<const RegisterId> WindowsX64ABI::get_callee_saved_fp_registers() const noexcept
{
    return winx64_fp_callee_saved;
}

RegisterId WindowsX64ABI::get_call_return_value_gp_register() const noexcept
{
    return GpRegisters_x86_64_RAX;
}

RegisterId WindowsX64ABI::get_call_return_value_fp_register() const noexcept
{
    return FpRegisters_x86_64_Xmm0;
}

std::span<const RegisterId> WindowsX64ABI::get_call_args_gp_registers() const noexcept
{
    return winx64_call_args_gp;
}

std::span<const RegisterId> WindowsX64ABI::get_call_args_fp_registers() const noexcept
{
    return winx64_call_args_fp;
}

std::uint64_t WindowsX64ABI::get_call_shadow_space_size() const noexcept
{
    return 32;
}

std::uint64_t WindowsX64ABI::get_stack_base_offset() const noexcept
{
    return 8;
}

/* LinuxX64ABI (SysV) */

std::string_view LinuxX64ABI::get_as_string() const noexcept
{
    return "Linux x64";
}

std::uint32_t LinuxX64ABI::get_id() const noexcept
{
    return PlatformABIID_LinuxX64;
}

ISA LinuxX64ABI::get_target_isa() const noexcept
{
    return ISA::x86_64;
}

bool LinuxX64ABI::can_fold_memory_operand() const noexcept
{
    return true;
}

bool LinuxX64ABI::has_two_address_form() const noexcept
{
    return true;
}

RegisterId LinuxX64ABI::get_variable_base_ptr() const noexcept
{
    return GpRegisters_x86_64_RDI;
}

RegisterId LinuxX64ABI::get_literal_base_ptr() const noexcept
{
    return GpRegisters_x86_64_RSI;
}

RegisterId LinuxX64ABI::get_function_call_ptr() const noexcept
{
    return GpRegisters_x86_64_RAX;
}

std::span<const RegisterId> LinuxX64ABI::get_gp_allocatable_registers() const noexcept
{
    return linuxx64_gp_allocatable;
}

std::span<const RegisterId> LinuxX64ABI::get_fp_allocatable_registers() const noexcept
{
    return linuxx64_fp_allocatable;
}

std::span<const RegisterId> LinuxX64ABI::get_caller_saved_gp_registers() const noexcept
{
    return linuxx64_gp_caller_saved;
}

std::span<const RegisterId> LinuxX64ABI::get_caller_saved_fp_registers() const noexcept
{
    return linuxx64_fp_caller_saved;
}

std::span<const RegisterId> LinuxX64ABI::get_callee_saved_gp_registers() const noexcept
{
    return linuxx64_gp_callee_saved;
}

std::span<const RegisterId> LinuxX64ABI::get_callee_saved_fp_registers() const noexcept
{
    return {};
}

RegisterId LinuxX64ABI::get_call_return_value_gp_register() const noexcept
{
    return GpRegisters_x86_64_RAX;
}

RegisterId LinuxX64ABI::get_call_return_value_fp_register() const noexcept
{
    return FpRegisters_x86_64_Xmm0;
}

std::span<const RegisterId> LinuxX64ABI::get_call_args_gp_registers() const noexcept
{
    return linuxx64_call_args_gp;
}

std::span<const RegisterId> LinuxX64ABI::get_call_args_fp_registers() const noexcept
{
    return linuxx64_call_args_fp;
}

std::uint64_t LinuxX64ABI::get_stack_red_zone_size() const noexcept
{
    return 128;
}

std::uint64_t LinuxX64ABI::get_stack_base_offset() const noexcept
{
    return 8;
}

/* AppleARM64ABI */

std::string_view AppleARM64ABI::get_as_string() const noexcept
{
    return "Apple ARM64";
}

std::uint32_t AppleARM64ABI::get_id() const noexcept
{
    return PlatformABIID_AppleARM64;
}

ISA AppleARM64ABI::get_target_isa() const noexcept
{
    return ISA::aarch64;
}

RegisterId AppleARM64ABI::get_variable_base_ptr() const noexcept
{
    return GpRegisters_aarch64_X0;
}

RegisterId AppleARM64ABI::get_literal_base_ptr() const noexcept
{
    return GpRegisters_aarch64_X1;
}

RegisterId AppleARM64ABI::get_function_call_ptr() const noexcept
{
    return GpRegisters_aarch64_X9;
}

std::span<const RegisterId> AppleARM64ABI::get_gp_allocatable_registers() const noexcept
{
    return arm64_gp_allocatable;
}

std::span<const RegisterId> AppleARM64ABI::get_fp_allocatable_registers() const noexcept
{
    return arm64_fp_allocatable;
}

std::span<const RegisterId> AppleARM64ABI::get_caller_saved_gp_registers() const noexcept
{
    return arm64_gp_caller_saved;
}

std::span<const RegisterId> AppleARM64ABI::get_caller_saved_fp_registers() const noexcept
{
    return arm64_fp_caller_saved;
}

std::span<const RegisterId> AppleARM64ABI::get_callee_saved_gp_registers() const noexcept
{
    return arm64_gp_callee_saved;
}

std::span<const RegisterId> AppleARM64ABI::get_callee_saved_fp_registers() const noexcept
{
    return arm64_fp_callee_saved;
}

RegisterId AppleARM64ABI::get_call_return_value_gp_register() const noexcept
{
    return GpRegisters_aarch64_X0;
}

RegisterId AppleARM64ABI::get_call_return_value_fp_register() const noexcept
{
    return FpRegisters_aarch64_V0;
}

std::span<const RegisterId> AppleARM64ABI::get_call_args_gp_registers() const noexcept
{
    return arm64_call_args_gp;
}

std::span<const RegisterId> AppleARM64ABI::get_call_args_fp_registers() const noexcept
{
    return arm64_call_args_fp;
}

std::uint64_t AppleARM64ABI::get_stack_red_zone_size() const noexcept
{
    return 128;
}

std::uint64_t AppleARM64ABI::get_stack_base_offset() const noexcept
{
    return 16; /* fp + lr pair */
}

/* NvPTXABI */

std::string_view NvPTXABI::get_as_string() const noexcept
{
    return "NvPTX";
}

std::uint32_t NvPTXABI::get_id() const noexcept
{
    return PlatformABIID_NvPTX;
}

ISA NvPTXABI::get_target_isa() const noexcept
{
    return ISA::nvptx;
}

bool NvPTXABI::has_virtual_registers() const noexcept
{
    return true;
}

RegisterId NvPTXABI::get_variable_base_ptr() const noexcept
{
    return INVALID_GP_REGISTER;
}

RegisterId NvPTXABI::get_literal_base_ptr() const noexcept
{
    return INVALID_GP_REGISTER;
}

RegisterId NvPTXABI::get_function_call_ptr() const noexcept
{
    return INVALID_GP_REGISTER;
}

std::span<const RegisterId> NvPTXABI::get_gp_allocatable_registers() const noexcept
{
    return {};
}

std::span<const RegisterId> NvPTXABI::get_fp_allocatable_registers() const noexcept
{
    return {};
}

std::span<const RegisterId> NvPTXABI::get_caller_saved_gp_registers() const noexcept
{
    return {};
}

std::span<const RegisterId> NvPTXABI::get_caller_saved_fp_registers() const noexcept
{
    return {};
}

std::span<const RegisterId> NvPTXABI::get_callee_saved_gp_registers() const noexcept
{
    return {};
}

std::span<const RegisterId> NvPTXABI::get_callee_saved_fp_registers() const noexcept
{
    return {};
}

RegisterId NvPTXABI::get_call_return_value_gp_register() const noexcept
{
    return INVALID_GP_REGISTER;
}

RegisterId NvPTXABI::get_call_return_value_fp_register() const noexcept
{
    return INVALID_FP_REGISTER;
}

std::span<const RegisterId> NvPTXABI::get_call_args_gp_registers() const noexcept
{
    return {};
}

std::span<const RegisterId> NvPTXABI::get_call_args_fp_registers() const noexcept
{
    return {};
}

MATHEXPR_NAMESPACE_END
