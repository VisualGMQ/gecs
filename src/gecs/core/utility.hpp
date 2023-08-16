#pragma once

#include <type_traits>

namespace gecs {

inline constexpr size_t IsPowerOf2(size_t number) {
    return number && ((number - 1) & number == 0);
}

template <typename T>
constexpr T QuickMod(T a, T mod) {
    static_assert(std::is_integral_v<T> && std::is_unsigned_v<T> && IsPowerOf2(mod));
    return a & (mod - 1);
}


//! @brief a small help class to show type info when compile
//! @tparam T 
template <typename T>
struct show_tmpl_error;

}