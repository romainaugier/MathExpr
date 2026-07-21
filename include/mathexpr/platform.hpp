// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHEXPR_PLATFORM)
#define __MATHEXPR_PLATFORM

#include "mathexpr/common.hpp"

#include <limits>
#include <vector>

MATHEXPR_NAMESPACE_BEGIN

enum Platform : uint32_t
{
    Platform_Windows,
    Platform_Linux,
    Platform_Apple,
    Platform_Invalid,
};

MATHEXPR_API uint32_t get_current_platform() noexcept;

MATHEXPR_API const char* platform_as_string(uint32_t platform) noexcept;

enum ISA : uint32_t
{
    ISA_x86_64,
    ISA_aarch64,
    ISA_Invalid,
};

MATHEXPR_API uint32_t get_current_isa() noexcept;

MATHEXPR_API const char* isa_as_string(uint32_t isa) noexcept;

using RegisterId = uint32_t;

static constexpr RegisterId INVALID_GP_REGISTER = std::numeric_limits<RegisterId>::max();
static constexpr RegisterId INVALID_FP_REGISTER = std::numeric_limits<RegisterId>::max();

/* x86_64 registers */

/* General Purpose Registers */
enum GpRegisters_x86_64 : RegisterId
{
    GpRegisters_x86_64_RAX,
    GpRegisters_x86_64_RBX,
    GpRegisters_x86_64_RCX,
    GpRegisters_x86_64_RDX,
    GpRegisters_x86_64_RSI,
    GpRegisters_x86_64_RDI,
    GpRegisters_x86_64_RBP,
    GpRegisters_x86_64_RSP,
    GpRegisters_x86_64_R8,
    GpRegisters_x86_64_R9,
    GpRegisters_x86_64_R10,
    GpRegisters_x86_64_R11,
    GpRegisters_x86_64_R12,
    GpRegisters_x86_64_R13,
    GpRegisters_x86_64_R14,
    GpRegisters_x86_64_R15,
};

/* Floating Point Registers (omitting the upper 8-15, not available on windows abi) */
enum FpRegisters_x86_64 : RegisterId
{
    FpRegisters_x86_64_Xmm0,
    FpRegisters_x86_64_Xmm1,
    FpRegisters_x86_64_Xmm2,
    FpRegisters_x86_64_Xmm3,
    FpRegisters_x86_64_Xmm4,
    FpRegisters_x86_64_Xmm5,
    FpRegisters_x86_64_Xmm6,
    FpRegisters_x86_64_Xmm7,
    FpRegisters_x86_64_Ymm0,
    FpRegisters_x86_64_Ymm1,
    FpRegisters_x86_64_Ymm2,
    FpRegisters_x86_64_Ymm3,
    FpRegisters_x86_64_Ymm4,
    FpRegisters_x86_64_Ymm5,
    FpRegisters_x86_64_Ymm6,
    FpRegisters_x86_64_Ymm7,
};

/* aarch64 registers */

enum GpRegisters_aarch64 : RegisterId
{
    GpRegisters_aarch64_X0,
    GpRegisters_aarch64_X1,
    GpRegisters_aarch64_X2,
    GpRegisters_aarch64_X3,
    GpRegisters_aarch64_X4,
    GpRegisters_aarch64_X5,
    GpRegisters_aarch64_X6,
    GpRegisters_aarch64_X7,
    GpRegisters_aarch64_X8,
    GpRegisters_aarch64_X9,
    GpRegisters_aarch64_X10,
    GpRegisters_aarch64_X11,
    GpRegisters_aarch64_X12,
    GpRegisters_aarch64_X13,
    GpRegisters_aarch64_X14,
    GpRegisters_aarch64_X15,
    GpRegisters_aarch64_X16,
    GpRegisters_aarch64_X17,
    GpRegisters_aarch64_X18,
    GpRegisters_aarch64_X19,
    GpRegisters_aarch64_X20,
    GpRegisters_aarch64_X21,
    GpRegisters_aarch64_X22,
    GpRegisters_aarch64_X23,
    GpRegisters_aarch64_X24,
    GpRegisters_aarch64_X25,
    GpRegisters_aarch64_X26,
    GpRegisters_aarch64_X27,
    GpRegisters_aarch64_X28,
    GpRegisters_aarch64_X29,
    GpRegisters_aarch64_X30,
};

enum FpRegisters_aarch64 : RegisterId
{
    FpRegisters_aarch64_V0,
    FpRegisters_aarch64_V1,
    FpRegisters_aarch64_V2,
    FpRegisters_aarch64_V3,
    FpRegisters_aarch64_V4,
    FpRegisters_aarch64_V5,
    FpRegisters_aarch64_V6,
    FpRegisters_aarch64_V7,
    FpRegisters_aarch64_V8,
    FpRegisters_aarch64_V9,
    FpRegisters_aarch64_V10,
    FpRegisters_aarch64_V11,
    FpRegisters_aarch64_V12,
    FpRegisters_aarch64_V13,
    FpRegisters_aarch64_V14,
    FpRegisters_aarch64_V15,
    FpRegisters_aarch64_V16,
    FpRegisters_aarch64_V17,
    FpRegisters_aarch64_V18,
    FpRegisters_aarch64_V19,
    FpRegisters_aarch64_V20,
    FpRegisters_aarch64_V21,
    FpRegisters_aarch64_V22,
    FpRegisters_aarch64_V23,
    FpRegisters_aarch64_V24,
    FpRegisters_aarch64_V25,
    FpRegisters_aarch64_V26,
    FpRegisters_aarch64_V27,
    FpRegisters_aarch64_V28,
    FpRegisters_aarch64_V29,
    FpRegisters_aarch64_V30,
    FpRegisters_aarch64_V31,
};

/* registers as string, convenient for pretty printing */
MATHEXPR_API const char* gp_register_as_string(RegisterId reg, uint32_t isa) noexcept;
MATHEXPR_API const char* fp_register_as_string(RegisterId reg, uint32_t isa) noexcept;

MATHEXPR_NAMESPACE_END

#endif /* !defined(__MATHEXPR_PLATFORM) */
