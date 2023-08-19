#pragma once

#include "gecs/config/config.hpp"
#include "gecs/core/type_list.hpp"

#include <utility>
#include <tuple>

namespace gecs {

/**
 * @brief point to a type can be mutable accessed
 * 
 * @tparam T 
 */
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

    querier_iterator operator+(difference_type step) noexcept {
        querier_iterator iter = *this;
        return iter += step;
    }

    querier_iterator operator-(difference_type step) noexcept {
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

template <bool B, typename T>
using add_const_conditional = std::conditional_t<B, const T, T>;

template <typename WorldT, typename Type>
using decay_storage_for_t = std::decay_t<typename WorldT::template storage_for_by_mutable_t<remove_mut_t<Type>>>; 

template <typename WorldT, typename Type>
using storage_for_with_constness_t = add_const_conditional<!is_mutable_v<Type>, decay_storage_for_t<WorldT, Type>>;

}

/**
 * @brief a help class for accessing entity and their components from basic_world
 * 
 * @tparam EntityT 
 * @tparam PageSize 
 * @tparam WorldT 
 * @tparam Types 
 */
template <typename EntityT, size_t PageSize, typename WorldT, typename... Types>
class basic_querier {
public:
    using query_types = type_list<Types...>;
    using pool_container = std::tuple<internal::storage_for_with_constness_t<WorldT, Types>*...>;
    using pool_container_reference = pool_container&;
    using iterator = internal::querier_iterator<EntityT, std::decay_t<decltype(std::declval<typename WorldT::pool_base_type>().packed())>, internal::storage_for_with_constness_t<WorldT, Types>*...>;
    using const_iterator = const iterator;
    using entity_container = typename WorldT::pool_base_type::packed_container_type;

    basic_querier(pool_container pools, const entity_container& entities) noexcept: pools_(pools), entities_(entities) { }
    basic_querier(pool_container pools, entity_container&& entities) noexcept: pools_(pools), entities_(std::move(entities)) { }

    auto& entities() const noexcept {
        return entities_;
    }

    iterator begin() noexcept {
        return iterator(pools_, entities_, entities_.size());
    }

    iterator end() noexcept {
        return iterator(pools_, entities_, 0);
    }

    auto size() const noexcept {
        return entities_.size();
    }

    bool empty() const noexcept {
        return size() == 0;
    }

private:
    pool_container pools_;
    entity_container entities_;
};

}