#pragma once

#include <type_traits>

namespace gecs {

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


//! @brief contains a bunch of types
//! @tparam ...Args 
template <typename... Args>
struct type_list {};

//! @brief provide a convinient way to index element in type_list
//! @tparam ...Args 
//! @tparam Index 
template <size_t Index, typename List>
struct type_list_elements;

template <size_t Index, typename First, typename... Args>
struct type_list_elements<Index, type_list<First, Args...>>: type_list_elements<Index - 1u, type_list<Args...>> {};

template <typename First, typename... Args>
struct type_list_elements<0u, type_list<First, Args...>> {
    using type = First;
};

template <size_t Index, typename List>
using type_list_elements_t = typename type_list_elements<Index, List>::type;

}