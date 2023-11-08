#pragma once

#include "gecs/config/config.hpp"
#include "gecs/core/type_list.hpp"
#include "gecs/core/ident.hpp"

#include <algorithm>
#include <tuple>
#include <utility>


namespace gecs {

// some query conditions

/**
 * @brief point to a type can be mutable accessed
 */
template <typename>
struct mut;

/**
 * @brief the entity don't has this component
 */
template <typename...>
struct without;

/**
 * @brief the entity only has this component
 */
template <typename...>
struct only;

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

template <typename T>
struct is_without {
    static constexpr bool value = false;
};

template <typename... Ts>
struct is_without<without<Ts...>> {
    static constexpr bool value = true;
};

template <typename T>
constexpr bool is_without_v = is_without<T>::value;

template <typename T>
struct is_only {
    static constexpr bool value = false;
};

template <typename... Ts>
struct is_only<only<Ts...>> {
    static constexpr bool value = true;
};

template <typename T>
constexpr bool is_only_v = is_only<T>::value;

template <typename T>
struct get_condition_inner_types;

template <typename... Ts>
struct get_condition_inner_types<type_list<Ts...>> {
    using type = type_list<Ts...>;
};

template <typename... Ts>
struct get_condition_inner_types<gecs::without<Ts...>> {
    using type = type_list<Ts...>;
};

template <typename... Ts>
struct get_condition_inner_types<gecs::only<Ts...>> {
    using type = type_list<Ts...>;
};

template <typename T>
using get_condition_inner_types_t = typename get_condition_inner_types<T>::type;

template <bool IsOnly, typename RequireTypeList, typename ForbidTypeList>
struct query_condition final {
    static constexpr bool is_only = IsOnly;
    using require_list = RequireTypeList;
    using forbid_list = ForbidTypeList;
};

template <typename List>
constexpr void check_condition_valid() {
    constexpr size_t only_qualifirer_num =
        list_filter_t<List, internal::is_only>::size;
    if constexpr (only_qualifirer_num == 1) {
        static_assert(List::size == 1, "the 'only' condition number must <= 1");
    }

    constexpr size_t without_qualifier_num =
        list_filter_t<List, internal::is_without>::size;
    static_assert(without_qualifier_num <= 1,
                  "the 'without' condition number must <= 1");
}

template <typename List>
constexpr auto get_query_condition() {
    check_condition_valid<List>();

    using only_list = list_filter_t<List, internal::is_only>;
    if constexpr (only_list::size > 0) {
        return query_condition<
            true, get_condition_inner_types_t<list_head_t<only_list>>,
            type_list<>>{};
    } else {
        using without_list = list_filter_t<List, internal::is_without>;
        using remain_list = list_filter_not_t<List, internal::is_without>;
        if constexpr (without_list::size == 0) {
            return query_condition<false, remain_list, type_list<>>{};
        } else {
            return query_condition<
                false, remain_list,
                get_condition_inner_types_t<list_head_t<without_list>>>{};
        }
    }
}

template <typename... Types>
using get_query_condition_t = decltype(get_query_condition<Types...>());

template <typename EntityT, typename PoolContainer>
struct check_satisfied_helper {
    template <typename T>
    bool operator()(EntityT entity, const PoolContainer& pools) const {
        using type = remove_mut_t<T>;
        size_t idx = component_id_generator::gen<type>();

        return (idx < pools.size()) && pools[idx]->contain(entity);
    }
};

template <typename T, typename... Ts, typename EntityT, typename PoolContainer, typename F>
bool check_condition(type_list<T, Ts...>, EntityT entity, const PoolContainer& pools, F f) {
    if constexpr (sizeof...(Ts) == 0)  {
        return f.template operator()<T>(entity, pools);
    } else {
        return f.template operator()<T>(entity, pools) &&
               check_condition<Ts...>(type_list<Ts...>{}, entity, pools, f);
    }
}

struct check_satisfied_one {
    template <typename T, typename EntityT, typename PoolContainer>
    bool operator()(EntityT entity, const PoolContainer& pools) const {
        using type = remove_mut_t<T>;
        size_t idx = component_id_generator::gen<type>();

        return idx < pools.size() && pools[idx]->contain(entity);
    }
};

struct check_without_satisfied_one {
    template <typename T, typename EntityT, typename PoolContainer>
    bool operator()(EntityT entity, const PoolContainer& pools) const {
        using type = remove_mut_t<T>;
        size_t idx = component_id_generator::gen<type>();

        return idx >= pools.size() || !pools[idx]->contain(entity);
    }
};

template <typename EntityT, typename PoolContainer, size_t N>
bool do_check_only_satisfied(EntityT entity, const PoolContainer& pools,
                const std::array<size_t, N>& requireIndices) {
    for (int i = 0; i < pools.size(); i++) {
        if (pools[i]->contain(entity)) {
            bool found = false;
            for (auto idx : requireIndices) {
                if (idx == i) {
                    found = true;
                    break;
                }
            }

            if (!found) {
                return false;
            }
        } else {
            for (auto idx : requireIndices) {
                if (idx == i) {
                    return false;
                }
            }
        }
    }

    return true;
}

template <typename... Ts, typename EntityT, typename PoolContainer>
bool check_only_satisfied(type_list<Ts...>, EntityT entity, const PoolContainer& pools) {
    std::array<size_t, sizeof...(Ts)> indices = {
        component_id_generator::gen<remove_mut_t<Ts>>()...};

    return do_check_only_satisfied(entity, pools, indices);
}

template <typename T, typename... Ts, typename EntityT, typename PoolContainer,
          typename F>
bool check_condition_list(type_list<T, Ts...>, EntityT entity,
                          const PoolContainer& pools, F f) {
    if constexpr (sizeof...(Ts) == 0) {
        return f.template operator()<T>(entity, pools);
    } else {
        return f.template operator()<T>(entity, pools) &&
               check_condition_list(type_list<Ts...>{}, entity, pools, f);
    }
}

template <typename EntityT, typename PoolContainer, typename F>
bool check_condition_list(type_list<>, EntityT entity,
                          const PoolContainer& pools, F f) {
    return true;
}

template <typename Condition, typename EntityT, typename PoolContainer>
bool check_condition(EntityT entity, const PoolContainer& pools) {
    if constexpr (Condition::is_only) {
        return check_only_satisfied(typename Condition::require_list{}, entity, pools);
    } else {
        return check_condition_list(typename Condition::require_list{}, entity,
                                    pools, check_satisfied_one{}) &&
               check_condition_list(typename Condition::forbid_list{}, entity,
                                    pools, check_without_satisfied_one{});
    }
}

template <typename T>
constexpr T min(T value) {
    return value;
}

//! @brief custom min function for finding list of literals
template <typename T, typename... Args>
constexpr T min(T first, Args... args) {
    T rest_min = min(args...);
    return (first < rest_min) ? first : rest_min;
}

template <typename EntityT, typename Container, typename PoolTypes>
class querier_iterator final {
public:
    using pool_container_type = PoolTypes;
    using container_type = Container;
    using entity_type = EntityT;

    using size_type = typename Container::size_type;
    using difference_type = typename Container::difference_type;
    using iterator_category = std::random_access_iterator_tag;

    querier_iterator(pool_container_type pools, const container_type& entities,
                     size_type offset) noexcept
        : pools_(pools), entities_{entities}, offset_(offset) {}

    auto operator*() noexcept {
        auto entity = entities_[index()];
        return std::apply(
            [&entity](auto*... pool) {
                return std::tuple_cat(
                    std::make_tuple(static_cast<entity_type>(entity)),
                    std::forward_as_tuple(
                        (*pool)[static_cast<EntityT>(entity)])...);
            },
            pools_);
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

    querier_iterator& operator++() noexcept { return --offset_, *this; }

    querier_iterator operator--(int) noexcept {
        querier_iterator copy = *this;
        return --(*this), copy;
    }

    querier_iterator& operator--() noexcept { return ++offset_, *this; }

    bool operator==(const querier_iterator& o) const noexcept {
        return o.entities_ == entities_ && o.offset_ == offset_ &&
               o.pools_ == pools_;
    }

    bool operator!=(const querier_iterator& o) const noexcept {
        return !(*this == o);
    }

private:
    container_type entities_;
    size_type offset_;
    pool_container_type pools_;

    size_type index() const noexcept { return offset_ - 1; }
};

template <bool B, typename T>
using add_const_conditional = std::conditional_t<B, const T, T>;

template <typename WorldT, typename Type>
using decay_storage_for_t = std::decay_t<
    typename WorldT::template storage_for_by_mutable_t<remove_mut_t<Type>>>;

template <typename WorldT, typename Type>
using storage_for_with_constness_t =
    add_const_conditional<!is_mutable_v<Type>,
                          decay_storage_for_t<WorldT, Type>>;

}  // namespace internal

/**
 * @brief a help class for accessing entity and their components from
 * basic_registry
 *
 * @tparam EntityT
 * @tparam PageSize
 * @tparam WorldT
 * @tparam Types
 */
template <typename EntityT, size_t PageSize, typename RegistryT,
          typename... Types>
class basic_querier {
public:
    using query_condition = type_list<Types...>;
    using query_types =
        typename internal::get_query_condition_t<query_condition>::require_list;
    using query_raw_types = list_foreach_t<query_types, internal::remove_mut>;

    template <typename T>
    struct storage_for_with_constness {
        using type = internal::storage_for_with_constness_t<RegistryT, T>*;
    };

    using pool_container = typelist_to_tuple_t<
        list_foreach_t<query_types, storage_for_with_constness>>;
    using pool_container_reference = pool_container&;
    using iterator = internal::querier_iterator<
        EntityT,
        std::decay_t<decltype(std::declval<typename RegistryT::pool_base_type>()
                                  .packed())>,
        pool_container>;
    using const_iterator = const iterator;
    using entity_container =
        typename RegistryT::pool_base_type::packed_container_type;
    using entity_type = EntityT;

    basic_querier(pool_container pools,
                  const entity_container& entities) noexcept
        : pools_(pools), entities_(entities) {}

    basic_querier(pool_container pools, entity_container&& entities) noexcept
        : pools_(pools), entities_(std::move(entities)) {}

    auto& entities() const noexcept { return entities_; }

    template <typename T, typename Compare>
    auto& sort_by(Compare cmp) {
        auto& pool = std::get<find_first_v<T, query_raw_types>>(pools_);
        std::sort(entities_.rbegin(), entities_.rend(),
                  [&pool, cmp](const auto& e1, const auto& e2) {
                      return cmp((*pool)[static_cast<entity_type>(e1)],
                                 (*pool)[static_cast<entity_type>(e2)]);
                  });
        return *this;
    }

    template <typename T>
    auto& sort_by() {
        sort_by<T>(std::less<T>{});
        return *this;
    }

    iterator begin() noexcept {
        return iterator(pools_, entities_, entities_.size());
    }

    iterator end() noexcept { return iterator(pools_, entities_, 0); }

    auto size() const noexcept { return entities_.size(); }

    bool empty() const noexcept { return size() == 0; }

private:
    pool_container pools_;
    entity_container entities_;
};

}  // namespace gecs
