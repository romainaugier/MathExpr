// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#include "mathexpr/platform.hpp"

MATHEXPR_NAMESPACE_BEGIN

uint32_t get_current_platform() noexcept
{
#if defined(MATHEXPR_WIN)
    return Platform_Windows;
#elif defined(MATHEXPR_LINUX)
    return Platform_Linux;
#elif defined(MATHEXPR_APPLE)
    return Platform_Apple;
#endif /* defined(MATHEXPR_WIN) */
    return Platform_Invalid;
}

const char* platform_as_string(uint32_t platform) noexcept
{
    switch(platform)
    {
        case Platform_Linux:
            return "Linux";
        case Platform_Windows:
            return "Windows";
        case Platform_Apple:
            return "MacOS";
        default:
            return "Unknown platform";
    }
}

uint32_t get_current_isa() noexcept
{
#if defined(MATHEXPR_X64)
    return ISA_x86_64;
#elif defined(MATHEXPR_AARCH64)
    return ISA_aarch64;
#endif /* defined(MATHEXPR_X64) */
    return ISA_Invalid;
}

const char* isa_as_string(uint32_t isa) noexcept
{
    switch(isa)
    {
        case ISA_x86_64:
            return "x86_64";
        case ISA_aarch64:
            return "aarch64";
        default:
            return "Unknown ISA";
    }
}

const char* gp_register_as_string(uint32_t reg, uint32_t isa) noexcept
{
    switch(isa)
    {
        case ISA_x86_64:
        {
            switch(reg)
            {
                case GpRegisters_x86_64_RAX: return "rax";
                case GpRegisters_x86_64_RBX: return "rbx";
                case GpRegisters_x86_64_RCX: return "rcx";
                case GpRegisters_x86_64_RDX: return "rdx";
                case GpRegisters_x86_64_RSI: return "rsi";
                case GpRegisters_x86_64_RDI: return "rdi";
                case GpRegisters_x86_64_RBP: return "rbp";
                case GpRegisters_x86_64_RSP: return "rsp";
                case GpRegisters_x86_64_R8: return "r8";
                case GpRegisters_x86_64_R9: return "r9";
                case GpRegisters_x86_64_R10: return "r10";
                case GpRegisters_x86_64_R11: return "r11";
                case GpRegisters_x86_64_R12: return "r12";
                case GpRegisters_x86_64_R13: return "r13";
                case GpRegisters_x86_64_R14: return "r14";
                case GpRegisters_x86_64_R15: return "r15";
                default: return "???";
            }

            break;
        }

        case ISA_aarch64:
        {
            switch(reg)
            {
                case GpRegisters_aarch64_X0: return "X0";
                case GpRegisters_aarch64_X1: return "X1";
                case GpRegisters_aarch64_X2: return "X2";
                case GpRegisters_aarch64_X3: return "X3";
                case GpRegisters_aarch64_X4: return "X4";
                case GpRegisters_aarch64_X5: return "X5";
                case GpRegisters_aarch64_X6: return "X6";
                case GpRegisters_aarch64_X7: return "X7";
                case GpRegisters_aarch64_X8: return "X8";
                case GpRegisters_aarch64_X9: return "X9";
                case GpRegisters_aarch64_X10: return "X10";
                case GpRegisters_aarch64_X11: return "X11";
                case GpRegisters_aarch64_X12: return "X12";
                case GpRegisters_aarch64_X13: return "X13";
                case GpRegisters_aarch64_X14: return "X14";
                case GpRegisters_aarch64_X15: return "X15";
                case GpRegisters_aarch64_X16: return "X16";
                case GpRegisters_aarch64_X17: return "X17";
                case GpRegisters_aarch64_X18: return "X18";
                case GpRegisters_aarch64_X19: return "X19";
                case GpRegisters_aarch64_X20: return "X20";
                case GpRegisters_aarch64_X21: return "X21";
                case GpRegisters_aarch64_X22: return "X22";
                case GpRegisters_aarch64_X23: return "X23";
                case GpRegisters_aarch64_X24: return "X24";
                case GpRegisters_aarch64_X25: return "X25";
                case GpRegisters_aarch64_X26: return "X26";
                case GpRegisters_aarch64_X27: return "X27";
                case GpRegisters_aarch64_X28: return "X28";
                case GpRegisters_aarch64_X29: return "X29";
                case GpRegisters_aarch64_X30: return "X30";
                default: return "???";
            }

            break;
        }

        default:
            return "???";
    }
}

const char* fp_register_as_string(uint32_t reg, uint32_t isa) noexcept
{
    switch(isa)
    {
        case ISA_x86_64:
        {
            switch(reg)
            {
                case FpRegisters_x86_64_Xmm0: return "xmm0";
                case FpRegisters_x86_64_Xmm1: return "xmm1";
                case FpRegisters_x86_64_Xmm2: return "xmm2";
                case FpRegisters_x86_64_Xmm3: return "xmm3";
                case FpRegisters_x86_64_Xmm4: return "xmm4";
                case FpRegisters_x86_64_Xmm5: return "xmm5";
                case FpRegisters_x86_64_Xmm6: return "xmm6";
                case FpRegisters_x86_64_Xmm7: return "xmm7";
                case FpRegisters_x86_64_Ymm0: return "ymm0";
                case FpRegisters_x86_64_Ymm1: return "ymm1";
                case FpRegisters_x86_64_Ymm2: return "ymm2";
                case FpRegisters_x86_64_Ymm3: return "ymm3";
                case FpRegisters_x86_64_Ymm4: return "ymm4";
                case FpRegisters_x86_64_Ymm5: return "ymm5";
                case FpRegisters_x86_64_Ymm6: return "ymm6";
                case FpRegisters_x86_64_Ymm7: return "ymm7";
                default: return "???";
            }
        }

        case ISA_aarch64:
        {
            switch(reg)
            {
                case FpRegisters_aarch64_V0: return "V0";
                case FpRegisters_aarch64_V1: return "V1";
                case FpRegisters_aarch64_V2: return "V2";
                case FpRegisters_aarch64_V3: return "V3";
                case FpRegisters_aarch64_V4: return "V4";
                case FpRegisters_aarch64_V5: return "V5";
                case FpRegisters_aarch64_V6: return "V6";
                case FpRegisters_aarch64_V7: return "V7";
                case FpRegisters_aarch64_V8: return "V8";
                case FpRegisters_aarch64_V9: return "V9";
                case FpRegisters_aarch64_V10: return "V10";
                case FpRegisters_aarch64_V11: return "V11";
                case FpRegisters_aarch64_V12: return "V12";
                case FpRegisters_aarch64_V13: return "V13";
                case FpRegisters_aarch64_V14: return "V14";
                case FpRegisters_aarch64_V15: return "V15";
                case FpRegisters_aarch64_V16: return "V16";
                case FpRegisters_aarch64_V17: return "V17";
                case FpRegisters_aarch64_V18: return "V18";
                case FpRegisters_aarch64_V19: return "V19";
                case FpRegisters_aarch64_V20: return "V20";
                case FpRegisters_aarch64_V21: return "V21";
                case FpRegisters_aarch64_V22: return "V22";
                case FpRegisters_aarch64_V23: return "V23";
                case FpRegisters_aarch64_V24: return "V24";
                case FpRegisters_aarch64_V25: return "V25";
                case FpRegisters_aarch64_V26: return "V26";
                case FpRegisters_aarch64_V27: return "V27";
                case FpRegisters_aarch64_V28: return "V28";
                case FpRegisters_aarch64_V29: return "V29";
                case FpRegisters_aarch64_V30: return "V30";
                case FpRegisters_aarch64_V31: return "V31";
                default: return "???";
            }
        }

        default:
            return "???";
    }
}

MATHEXPR_NAMESPACE_END
