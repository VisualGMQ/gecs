#pragma once

#include <cstring>
#include <type_traits>

namespace gecs {

template <typename... Ts>
struct type_list {
    using type = type_list;
    static constexpr size_t size = sizeof...(Ts);
};


template <size_t, typename>
struct type_list_element;

template<typename First, typename... Remains>
struct type_list_element<0, type_list<First, Remains...>> {
    using type = First;
};

template <size_t Idx, typename First, typename... Remains>
struct type_list_element<Idx, type_list<First, Remains...>> : type_list_element<Idx - 1u, type_list<Remains...>> { };

template <size_t Idx, typename List>
using type_list_element_t = typename type_list_element<Idx, List>::type;


template <size_t Idx, size_t Max, typename T, typename List>
struct find_first;

template <size_t Idx, size_t Max, typename T, typename... Ts>
struct find_first<Idx, Max, T, type_list<Ts...>> {
    static constexpr size_t value = std::is_same_v<T, type_list_element_t<Idx, type_list<Ts...>>> ? Idx : find_first<Idx + 1, Max, T, type_list<Ts...>>::value;
};

template <size_t Max, typename T, typename... Ts>
struct find_first<Max, Max, T, type_list<Ts...>> {
    static constexpr size_t value = -1;
};

template <typename T, typename List>
constexpr size_t find_first_v = find_first<0, List::size, T, List>::value;

}