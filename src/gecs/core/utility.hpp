#pragma once

#include <type_traits>
#include <cassert>
#include <cstddef>
#include <utility>

namespace gecs {

#define ECS_ASSERT(expr, x) assert(((void)expr, x))

inline constexpr size_t is_power_of_2(size_t number) {
    return number && ((number - 1) & (number == 0));
}

template <typename T>
constexpr T quick_mod(T a, T mod) {
    static_assert(std::is_integral_v<T> && std::is_unsigned_v<T> && is_power_of_2(mod));
    return a & (mod - 1);
}


//! @brief a small help class to show type info when compile
//! @tparam T 
template <typename T>
struct show_tmpl_error;


//! @brief transcribe the constness from From to To
//! @tparam To 
//! @tparam From 
template <typename To, typename From>
struct constness_as {
    using type = std::remove_const_t<To>;
};

template <typename To, typename From>
struct constness_as<To, const From> {
    using type = const To;
};

template <typename To, typename From>
using constness_as_t = typename constness_as<To, From>::type;


template <typename T>
struct strip_function_pointer_to_type;

template <typename Ret, typename... Args>
struct strip_function_pointer_to_type<Ret(*)(Args...)> {
    using type = Ret(Args...);
};

template <typename Ret, typename... Args>
struct strip_function_pointer_to_type<Ret(Args...)> {
    using type = Ret(Args...);
};

template <typename T>
using strip_function_pointer_to_type_t = typename strip_function_pointer_to_type<T>::type;


template<std::size_t N, std::size_t... Seq>
constexpr std::index_sequence<N + Seq ...> add(std::index_sequence<Seq...>) { return {}; }

//! @brief make a std::index_sequence from a give range
//! @tparam Min 
//! @tparam Max 
template <std::size_t Min, std::size_t Max>
using make_index_range = decltype(add<Min>(std::make_index_sequence<Max - Min>()));

}