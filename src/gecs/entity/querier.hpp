#pragma once

#include "world.hpp"
#include <utility>
#include <tuple>

namespace gecs {

template <typename EntityT, size_t PageSize, typename... Types>
class basic_querier;

template <typename T>
struct mut;

namespace internal {

template <typename T>
struct is_mutable {
    static constexpr bool value = false;
};

template <typename T>
struct is_mutable<mut<T>> {
    static constexpr bool value = true;
};

template <typename T>
using is_mutable_v = is_mutable<T>::value;


template <typename T>
struct real_type {
    using type = T;
};

template <typename T>
struct real_type<mut<T>> {
    using type = T;
};

template <typename T>
using real_type_t = typename real_type<T>::type;


template<typename T>
constexpr T min(T value) {
    return value;
}

//! @brief custom min function for finding list of literals
template<typename T, typename... Args>
constexpr T min(T first, Args... args) {
    T rest_min = min(args...);
    return (first < rest_min) ? first : rest_min;
}


template <typename EntityT, typename... Types>
class querier_iterator final {
public:
    using pool_container_type = std::tuple<Types...>;
    using entity_container = std::vector<EntityT>;
    using value_type = std::tuple<std::conditional_t<std::is_const_v<Types>, const typename Types::value_type&, typename Types::value_type&>, ...>;
    using size_type = typename entity_container::size_type;

    querier_iterator(pool_container_type pools, const entity_container& entities, entity_container::size_type offset): pools_(pools), entities_{entities}, offset_(offset) {
        offset_ = entities_.size();
    }

    value_type operator*() noexcept {
        // TODO: not finish
    }

private:
    const entity_container& entities_;
    entity_container::size_type offset_;
    pool_container_type pools_;

    entity_container::size_type index() const noexcept {
        return offset_ - 1;
    }
};

}

template <typename EntityT, size_t PageSize, typename Type>
class basic_querier<EntityT, PageSize, Type> final {
public:
    using raw_pool_type = internal::storage_for_t<world_type::pool_basic_type, Type>;
    using pool_type = std::tuple<std::conditional_t<internal::is_mutable_v<Type>, raw_pool_type*, const raw_pool_type*>>;
    using pool_type_reference = pool_type&;
    using iterator = internal::querier_iterator<pool_type_reference>;
    using const_iterator = const iterator;

    basic_querier(pool_type pool): pool_(pool) noexcept { }

    auto& entities() const noexcept {
        return pool_.packed();
    }

    iterator begin() {
        return iterator{pool_, std::get<0>(pool_).packed(), std::get<0>(pool_).size()};
    }

    iterator end() {
        return iterator{pool_, std::get<0>(pool_).packed(), 0};
    }

private:
    using world_type = basic_world<EntityT, PageSize>;

    pool_type pool_;

    template <size_t... Indices>
    size_t minimal_list_idx(pool_type_reference& pools, std::index_sequence<Indices...>) {
        return internal::min<size_t>(std::get<Indices>(pools).size(), ...);
    }
};

}