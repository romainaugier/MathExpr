
#include "mathexpr/types.hpp"

MATHEXPR_NAMESPACE_BEGIN

std::size_t scalar_type_size(const ScalarType type) noexcept
{
    return (static_cast<std::size_t>(type) + 1) * 4;
}

MATHEXPR_NAMESPACE_END