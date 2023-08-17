#pragma once

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

}