// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHEXPR_X86_64)
#define __MATHEXPR_X86_64

#include "mathexpr/common.hpp"

#define X86_64_NAMESPACE_BEGIN namespace x86_64 {
#define X86_64_NAMESPACE_END }

MATHEXPR_NAMESPACE_BEGIN

X86_64_NAMESPACE_BEGIN

/* Helpers notes */

/*
    https://www.cs.uaf.edu/2002/fall/cs301/Encoding%20instructions.htm
*/

/* Register Codes (ModR/M, SIB fields) */
static constexpr std::byte RAX = static_cast<std::byte>(0);  // 000
static constexpr std::byte RCX = static_cast<std::byte>(1);  // 001
static constexpr std::byte RDX = static_cast<std::byte>(2);  // 010
static constexpr std::byte RBX = static_cast<std::byte>(3);  // 011
static constexpr std::byte RSP = static_cast<std::byte>(4);  // 100 (implies SIB when used as base)
static constexpr std::byte RBP = static_cast<std::byte>(5);  // 101 (requires disp32 if Mod == 00)
static constexpr std::byte RSI = static_cast<std::byte>(6);  // 110
static constexpr std::byte RDI = static_cast<std::byte>(7);  // 111

static constexpr std::byte R8 = static_cast<std::byte>(0);  // Use REX.B/R/X = 1
static constexpr std::byte R9 = static_cast<std::byte>(1);
static constexpr std::byte R10 = static_cast<std::byte>(2);
static constexpr std::byte R11 = static_cast<std::byte>(3);
static constexpr std::byte R12 = static_cast<std::byte>(4);
static constexpr std::byte R13 = static_cast<std::byte>(5);
static constexpr std::byte R14 = static_cast<std::byte>(6);
static constexpr std::byte R15 = static_cast<std::byte>(7);

static constexpr std::byte XMM0 = static_cast<std::byte>(0);
static constexpr std::byte XMM1 = static_cast<std::byte>(1);
static constexpr std::byte XMM2 = static_cast<std::byte>(2);
static constexpr std::byte XMM3 = static_cast<std::byte>(3);
static constexpr std::byte XMM4 = static_cast<std::byte>(4);
static constexpr std::byte XMM5 = static_cast<std::byte>(5);
static constexpr std::byte XMM6 = static_cast<std::byte>(6);
static constexpr std::byte XMM7 = static_cast<std::byte>(7);

/* REX Prefix (binary: 0100WRXB) */
static constexpr std::byte REX_BASE = static_cast<std::byte>(0x40);
static constexpr std::byte REX_W = static_cast<std::byte>(0x08);  // 64-bit operand
static constexpr std::byte REX_R = static_cast<std::byte>(0x04);  // Extension of Reg field
static constexpr std::byte REX_X = static_cast<std::byte>(0x02);  // Extension of Index field (SIB)
static constexpr std::byte REX_B = static_cast<std::byte>(0x01);  // Extension of R/M field or base

/*
    OPSD common opcodes (just as a reminder)

    movs
    MOVSD_LOAD  0xF2, 0x0F, 0x10   xmm, [mem]
    MOVSD_STORE 0xF2, 0x0F, 0x11   [mem], xmm

    binops
    ADDSD       0xF2, 0x0F, 0x58   xmm, xmm/mem
    SUBSD       0xF2, 0x0F, 0x5C
    MULSD       0xF2, 0x0F, 0x59
    DIVSD       0xF2, 0x0F, 0x5E

    unops
    SQRTSD      0xF2, 0x0F, 0x51

    terminators
    RET         0xC3               return
*/

/*
    ModR/M Byte (binary: mmrrrmmm)
    mod:   2 bits - 00=no disp, 01=disp8, 10=disp32, 11=register
    reg:   3 bits - XMM or GPR (source or opcode extension)
    r/m:   3 bits - base register or destination
*/

// Mod values
static constexpr std::byte MOD_INDIRECT = static_cast<std::byte>(0x00);  // [reg]
static constexpr std::byte MOD_INDIRECT_DISP8 = static_cast<std::byte>(0x40);  // [reg + imm8]
static constexpr std::byte MOD_INDIRECT_DISP32 = static_cast<std::byte>(0x80);  // [reg + imm32]
static constexpr std::byte MOD_DIRECT = static_cast<std::byte>(0xC0);  // Register to register

// SIB Byte (scale-index-base) if R/M == 100
// Needed if base == RSP | R12 or using scaled index
// SIB = (scale << 6) | (index << 3) | base
// scale: 00=1, 01=2, 10=4, 11=8

X86_64_NAMESPACE_END

MATHEXPR_NAMESPACE_END

#endif /* !defined(__MATHEXPR_X86_64) */
