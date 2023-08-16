#pragma once

#include <utility>
#include <tuple>
#include "gecs/config/config.hpp"

namespace gecs {

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
constexpr bool is_mutable_v = is_mutable<T>::value;


template <typename T>
struct remove_mut {
    using type = T;
};

template <typename T>
struct remove_mut<mut<T>> {
    using type = T;
};

template <typename T>
using remove_mut_t = typename remove_mut<T>::type;


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


template <typename EntityT, typename Container, typename... PoolTypes>
class querier_iterator final {
public:
    using pool_container_type = std::tuple<PoolTypes...>;
    using container_type = Container;
    using entity_type = EntityT;

    using size_type = typename Container::size_type;
    using difference_type = typename Container::difference_type;
    using iterator_category = std::random_access_iterator_tag;

    querier_iterator(pool_container_type pools, const container_type& entities, size_type offset) noexcept: pools_(pools), entities_{&entities}, offset_(offset) { }

    auto operator*() noexcept {
        auto entity = (*entities_)[index()];
        return std::apply([&entity](auto*... pool){
            return std::tuple_cat(std::make_tuple(entity), std::forward_as_tuple((*pool)[static_cast<EntityT>(entity)])...);
        }, pools_);
    }

    querier_iterator& operator+=(difference_type step) noexcept {
        return offset_ -= step, *this;
    }

    querier_iterator& operator-=(difference_type step) noexcept {
        return operator+=(-step);
    }

    querier_iterator& operator+(difference_type step) noexcept {
        querier_iterator iter = *this;
        return iter += step;
    }

    querier_iterator& operator-(difference_type step) noexcept {
        return operator+(-step);
    }

    querier_iterator operator++(int) noexcept {
        querier_iterator copy = *this;
        return ++(*this), copy;
    }

    querier_iterator& operator++() noexcept {
        return --offset_, *this;
    }

    querier_iterator operator--(int) noexcept {
        querier_iterator copy = *this;
        return --(*this), copy;
    }

    querier_iterator& operator--() noexcept {
        return ++offset_, *this;
    }

    bool operator==(const querier_iterator& o) const noexcept {
        return o.entities_ == entities_ && o.offset_ == offset_ && o.pools_ == pools_;
    }

    bool operator!=(const querier_iterator& o) const noexcept {
        return !(*this == o);
    }

private:
    const container_type* entities_;
    size_type offset_;
    pool_container_type pools_;

    size_type index() const noexcept {
        return offset_ - 1;
    }
};

}


template <typename EntityT, size_t PageSize, typename WorldT, typename... PoolTypes>
class basic_querier;

template <typename EntityT, size_t PageSize, typename WorldT, typename Type>
class basic_querier<EntityT, PageSize, WorldT, Type> final {
public:
    using query_types = std::tuple<Type>;
    using pool_type = std::decay_t<typename WorldT::template storage_for_by_mutable_t<internal::remove_mut_t<Type>>>;
    using pool_type_with_constness = std::conditional_t<internal::is_mutable_v<Type>, pool_type, const pool_type>;
    using pool_type_pointer_with_constness = pool_type_with_constness *;
    using pool_container = std::tuple<pool_type_pointer_with_constness>;
    using pool_container_reference = pool_container&;
    using iterator = internal::querier_iterator<EntityT, std::decay_t<decltype(std::declval<WorldT::pool_base_type>().packed())>, pool_type_pointer_with_constness>;
    using const_iterator = const iterator;

    basic_querier(pool_container pool) noexcept: pool_(pool) { }

    auto& entities() const noexcept {
        return pool_.packed();
    }

    iterator begin() noexcept {
        auto& packed = std::get<0>(pool_)->packed();
        return iterator(pool_, packed, std::get<0>(pool_)->size());
    }

    iterator end() noexcept {
        auto& packed = std::get<0>(pool_)->packed();
        return iterator(pool_, packed, 0);
    }

    auto size() const {
        return std::get<0>(pool_)->packed().size();
    }

private:
    pool_container pool_;

    template <size_t... Indices>
    size_t minimal_list_idx(pool_container_reference& pools, std::index_sequence<Indices...>) {
        return internal::min<size_t>(std::get<Indices>(pools).size(), ...);
    }
};

}