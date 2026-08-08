// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#include "mathexpr/libmaths.hpp"
#include "mathexpr/string_hash.hpp"

#include <cmath>
#include <array>

MATHEXPR_NAMESPACE_BEGIN

LIBMATHS_NAMESPACE_BEGIN

/* Functions implementation */

/* Core mathematical functions */
/* Absolute value */
double abs_d(const double x) noexcept
{
    return ::fabs(x);
}

double2 abs_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

#if defined(MATHEXPR_X86_64)
double4 abs_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}
#endif // defined(MATHEXPR_X86_64)

/* Square root */
double sqrt_d(const double x) noexcept
{
    return ::sqrt(x);
}

double2 sqrt_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

#if defined(MATHEXPR_X86_64)
double4 sqrt_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}
#endif // defined(MATHEXPR_X86_64)

/* Cube root */
double cbrt_d(const double x) noexcept
{
    return ::cbrt(x);
}

double2 cbrt_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

#if defined(MATHEXPR_X86_64)
double4 cbrt_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}
#endif // defined(MATHEXPR_X86_64)

/* Power function */
double pow_d(const double x, const double y) noexcept
{
    return ::pow(x, y);
}

double2 pow_d2(const double2 x, const double2 y) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

#if defined(MATHEXPR_X86_64)
double4 pow_d4(const double4 x, const double4 y) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}
#endif // defined(MATHEXPR_X86_64)

/* Exponential function */
double exp_d(const double x) noexcept
{
    return ::exp(x);
}

double2 exp_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

#if defined(MATHEXPR_X86_64)
double4 exp_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}
#endif // defined(MATHEXPR_X86_64)

/* exp(x) - 1 */
double expm1_d(const double x) noexcept
{
    return ::expm1(x);
}

double2 expm1_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

#if defined(MATHEXPR_X86_64)
double4 expm1_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}
#endif // defined(MATHEXPR_X86_64)

/* Natural logarithm */
double log_d(const double x) noexcept
{
    return ::log(x);
}

double2 log_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

#if defined(MATHEXPR_X86_64)
double4 log_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}
#endif // defined(MATHEXPR_X86_64)

/* Base-10 logarithm */
double log10_d(const double x) noexcept
{
    return log10(x);
}

double2 log10_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

#if defined(MATHEXPR_X86_64)
double4 log10_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}
#endif // defined(MATHEXPR_X86_64)

/* Base-2 logarithm */
double log2_d(const double x) noexcept
{
    return log2(x);
}

double2 log2_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

#if defined(MATHEXPR_X86_64)
double4 log2_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}
#endif // defined(MATHEXPR_X86_64)

/* log(1 + x) */
double log1p_d(const double x) noexcept
{
    return ::log1p(x);
}

double2 log1p_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

#if defined(MATHEXPR_X86_64)
double4 log1p_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}
#endif // defined(MATHEXPR_X86_64)

/* Trigonometric functions */
/* Sine */
double sin_d(const double x) noexcept
{
    return ::sin(x);
}

double2 sin_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

#if defined(MATHEXPR_X86_64)
double4 sin_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}
#endif // defined(MATHEXPR_X86_64)

/* Cosine */
double cos_d(const double x) noexcept
{
    return ::cos(x);
}

double2 cos_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

#if defined(MATHEXPR_X86_64)
double4 cos_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}
#endif // defined(MATHEXPR_X86_64)

/* Tangent */
double tan_d(const double x) noexcept
{
    return ::tan(x);
}

double2 tan_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

#if defined(MATHEXPR_X86_64)
double4 tan_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}
#endif // defined(MATHEXPR_X86_64)

/* Arcsine */
double asin_d(const double x) noexcept
{
    return ::asin(x);
}

double2 asin_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

#if defined(MATHEXPR_X86_64)
double4 asin_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}
#endif // defined(MATHEXPR_X86_64)

/* Arccosine */
double acos_d(const double x) noexcept
{
    return ::acos(x);
}

double2 acos_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

#if defined(MATHEXPR_X86_64)
double4 acos_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}
#endif // defined(MATHEXPR_X86_64)

/* Arctangent */
double atan_d(const double x) noexcept
{
    return ::atan(x);
}

double2 atan_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

#if defined(MATHEXPR_X86_64)
double4 atan_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}
#endif // defined(MATHEXPR_X86_64)

/* Arctangent with two arguments */
double atan2_d(const double y, const double x) noexcept
{
    return ::atan2(y, x);
}

double2 atan2_d2(const double2 y, const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

#if defined(MATHEXPR_X86_64)
double4 atan2_d4(const double4 y, const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}
#endif // defined(MATHEXPR_X86_64)

/* Hyperbolic functions */
/* Hyperbolic sine */
double sinh_d(const double x) noexcept
{
    return ::sinh(x);
}

double2 sinh_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

#if defined(MATHEXPR_X86_64)
double4 sinh_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}
#endif // defined(MATHEXPR_X86_64)

/* Hyperbolic cosine */
double cosh_d(const double x) noexcept
{
    return ::cosh(x);
}

double2 cosh_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

#if defined(MATHEXPR_X86_64)
double4 cosh_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}
#endif // defined(MATHEXPR_X86_64)

/* Hyperbolic tangent */
double tanh_d(const double x) noexcept
{
    return ::tanh(x);
}

double2 tanh_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

#if defined(MATHEXPR_X86_64)
double4 tanh_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}
#endif // defined(MATHEXPR_X86_64)

/* Inverse hyperbolic sine */
double asinh_d(const double x) noexcept
{
    return asinh(x);
}

double2 asinh_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

#if defined(MATHEXPR_X86_64)
double4 asinh_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}
#endif // defined(MATHEXPR_X86_64)

/* Inverse hyperbolic cosine */
double acosh_d(const double x) noexcept
{
    return ::acosh(x);
}

double2 acosh_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

#if defined(MATHEXPR_X86_64)
double4 acosh_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}
#endif // defined(MATHEXPR_X86_64)

/* Inverse hyperbolic tangent */
double atanh_d(const double x) noexcept
{
    return ::atanh(x);
}

double2 atanh_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

#if defined(MATHEXPR_X86_64)
double4 atanh_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}
#endif // defined(MATHEXPR_X86_64)

/* Rounding and modulo */
/* Floor function */
double floor_d(const double x) noexcept
{
    return ::floor(x);
}

double2 floor_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

#if defined(MATHEXPR_X86_64)
double4 floor_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}
#endif // defined(MATHEXPR_X86_64)

/* Ceiling function */
double ceil_d(const double x) noexcept
{
    return ::ceil(x);
}

double2 ceil_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

#if defined(MATHEXPR_X86_64)
double4 ceil_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}
#endif // defined(MATHEXPR_X86_64)

/* Truncate */
double trunc_d(const double x) noexcept
{
    return ::trunc(x);
}

double2 trunc_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

#if defined(MATHEXPR_X86_64)
double4 trunc_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}
#endif // defined(MATHEXPR_X86_64)

/* Round to nearest */
double round_d(const double x) noexcept
{
    return ::round(x);
}

double2 round_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

#if defined(MATHEXPR_X86_64)
double4 round_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}
#endif // defined(MATHEXPR_X86_64)

/* Floating-point remainder */
double fmod_d(const double x, const double y) noexcept
{
    return ::fmod(x, y);
}

double2 fmod_d2(const double2 x, const double2 y) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

#if defined(MATHEXPR_X86_64)
double4 fmod_d4(const double4 x, const double4 y) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}
#endif // defined(MATHEXPR_X86_64)

/* IEEE remainder */
double remainder_d(const double x, const double y) noexcept
{
    return ::remainder(x, y);
}

double2 remainder_d2(const double2 x, const double2 y) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

#if defined(MATHEXPR_X86_64)
double4 remainder_d4(const double4 x, const double4 y) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}
#endif // defined(MATHEXPR_X86_64)

/* Copy sign from y to x */
double copysign_d(const double x, const double y) noexcept
{
    return copysign(x, y);
}

double2 copysign_d2(const double2 x, const double2 y) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

#if defined(MATHEXPR_X86_64)
double4 copysign_d4(const double4 x, const double4 y) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}
#endif // defined(MATHEXPR_X86_64)

/* Miscellaneous */
/* Hypotenuse sqrt(x*x + y*y) */
double hypot_d(const double x, const double y) noexcept
{
    return hypot(x, y);
}

double2 hypot_d2(const double2 x, const double2 y) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

#if defined(MATHEXPR_X86_64)
double4 hypot_d4(const double4 x, const double4 y) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}
#endif // defined(MATHEXPR_X86_64)

/* Convert degrees to radians */
double radians_d(const double x) noexcept
{
    return x;
}

double2 radians_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

#if defined(MATHEXPR_X86_64)
double4 radians_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}
#endif // defined(MATHEXPR_X86_64)

/* Convert radians to degrees */
double degrees_d(const double x) noexcept
{
    return x;
}

double2 degrees_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

#if defined(MATHEXPR_X86_64)
double4 degrees_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}
#endif // defined(MATHEXPR_X86_64)

/* Function table and map */

static constexpr std::size_t NUM_FUNCTIONS = static_cast<std::size_t>(FunctionId::COUNT);

#if defined(MATHEXPR_X86_64)
#define REGISTER_FUNCTION(base, arity)                                    \
    {                                                                     \
        MATHEXPR_STRIFY(base),                                            \
        reinterpret_cast<void*>(static_cast<Fn##arity##_d>(&base##_d)),   \
        reinterpret_cast<void*>(static_cast<Fn##arity##_d2>(&base##_d2)), \
        reinterpret_cast<void*>(static_cast<Fn##arity##_d4>(&base##_d4)), \
        arity                                                             \
    }
#elif defined(MATHEXPR_AARCH64)
#define REGISTER_FUNCTION(base, arity)                                    \
    {                                                                     \
        MATHEXPR_STRIFY(base),                                            \
        reinterpret_cast<void*>(static_cast<Fn##arity##_d>(&base##_d)),   \
        reinterpret_cast<void*>(static_cast<Fn##arity##_d2>(&base##_d2)), \
        arity                                                             \
    } 
#endif // defined(MATHEXPR_X86_64)

using FuncTable = std::array<const FunctionEntry, NUM_FUNCTIONS>;

using FuncMap = std::unordered_map<std::string_view, FunctionId, string_hash, std::equal_to<>>;

static const FuncTable g_function_table = {{
    REGISTER_FUNCTION(abs, 1),
    REGISTER_FUNCTION(sqrt, 1),
    REGISTER_FUNCTION(cbrt, 1),
    REGISTER_FUNCTION(pow, 2),
    REGISTER_FUNCTION(exp, 1),
    REGISTER_FUNCTION(expm1, 1),
    REGISTER_FUNCTION(log, 1),
    REGISTER_FUNCTION(log10, 1),
    REGISTER_FUNCTION(log2, 1),
    REGISTER_FUNCTION(log1p, 1),
    REGISTER_FUNCTION(sin, 1),
    REGISTER_FUNCTION(cos, 1),
    REGISTER_FUNCTION(tan, 1),
    REGISTER_FUNCTION(asin, 1),
    REGISTER_FUNCTION(acos, 1),
    REGISTER_FUNCTION(atan, 1),
    REGISTER_FUNCTION(atan2, 2),
    REGISTER_FUNCTION(sinh, 1),
    REGISTER_FUNCTION(cosh, 1),
    REGISTER_FUNCTION(tanh, 1),
    REGISTER_FUNCTION(asinh, 1),
    REGISTER_FUNCTION(acosh, 1),
    REGISTER_FUNCTION(atanh, 1),
    REGISTER_FUNCTION(floor, 1),
    REGISTER_FUNCTION(ceil, 1),
    REGISTER_FUNCTION(trunc, 1),
    REGISTER_FUNCTION(fmod, 2),
    REGISTER_FUNCTION(remainder, 2),
    REGISTER_FUNCTION(copysign, 2),
    REGISTER_FUNCTION(hypot, 2),
    REGISTER_FUNCTION(radians, 1),
    REGISTER_FUNCTION(degrees, 1),
}};

#define MAP_FUNCTION(name, id) { name, id } 

static const FuncMap g_function_map = {
    MAP_FUNCTION("abs", FunctionId::Abs),
    MAP_FUNCTION("sqrt", FunctionId::Sqrt),
    MAP_FUNCTION("cbrt", FunctionId::Cbrt),
    MAP_FUNCTION("pow", FunctionId::Pow),
    MAP_FUNCTION("exp", FunctionId::Exp),
    MAP_FUNCTION("expm1", FunctionId::Expm1),
    MAP_FUNCTION("log", FunctionId::Log),
    MAP_FUNCTION("log10", FunctionId::Log10),
    MAP_FUNCTION("log2", FunctionId::Log2),
    MAP_FUNCTION("log1p", FunctionId::Log1p),
    MAP_FUNCTION("sin", FunctionId::Sin),
    MAP_FUNCTION("cos", FunctionId::Cos),
    MAP_FUNCTION("tan", FunctionId::Tan),
    MAP_FUNCTION("asin", FunctionId::Asin),
    MAP_FUNCTION("acos", FunctionId::Acos),
    MAP_FUNCTION("atan", FunctionId::Atan),
    MAP_FUNCTION("atan2", FunctionId::Atan2),
    MAP_FUNCTION("sinh", FunctionId::Sinh),
    MAP_FUNCTION("cosh", FunctionId::Cosh),
    MAP_FUNCTION("tanh", FunctionId::Tanh),
    MAP_FUNCTION("asinh", FunctionId::Asinh),
    MAP_FUNCTION("acosh", FunctionId::Acosh),
    MAP_FUNCTION("atanh", FunctionId::Atanh),
    MAP_FUNCTION("floor", FunctionId::Floor),
    MAP_FUNCTION("ceil", FunctionId::Ceil),
    MAP_FUNCTION("trunc", FunctionId::Trunc),
    MAP_FUNCTION("fmod", FunctionId::Fmod),
    MAP_FUNCTION("remainder", FunctionId::Remainder),
    MAP_FUNCTION("copysign", FunctionId::Copysign),
    MAP_FUNCTION("hypot", FunctionId::Hypot),
    MAP_FUNCTION("radians", FunctionId::Radians),
    MAP_FUNCTION("degrees", FunctionId::Degrees),
};

FunctionId get_function_id(std::string_view name) noexcept
{
    auto entry = g_function_map.find(name);

    if(entry == g_function_map.end())
        return FunctionId::Unknown;

    return entry->second;
}

const FunctionEntry* get_function_entry(std::string_view name) noexcept
{
    auto entry = g_function_map.find(name);

    if(entry == g_function_map.end())
        return nullptr;

    return std::addressof(g_function_table[static_cast<std::size_t>(entry->second)]);
}

const FunctionEntry* get_function_entry(FunctionId id) noexcept
{
    if(id >= FunctionId::COUNT)
        return nullptr;

    return std::addressof(g_function_table[static_cast<std::size_t>(id)]);
}

LIBMATHS_NAMESPACE_END

MATHEXPR_NAMESPACE_END
