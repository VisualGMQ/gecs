#pragma once

#include "config.hpp"

#include <type_traits>

namespace gecs {

namespace internal {

#define ECS_ASSERT(x) assert(x)

inline constexpr uint32_t PageSize = 4096;

template<typename Type>
static constexpr int popcount(Type value) noexcept {
    return value ? (int(value & 1) + popcount(value >> 1)) : 0;
}

template <typename EntityT, typename = void>
struct entity_traits;

template <typename EntityT>
struct entity_traits<EntityT, std::enable_if_t<std::is_enum_v<EntityT>>>: public entity_traits<std::underlying_type_t<EntityT>> {};

template <>
struct entity_traits<uint32_t> {
    using entity_type = uint32_t;

    using entity_mask_type = uint32_t;
    using version_mask_type = uint16_t;

    static constexpr uint32_t entity_mask = 0xFFFFF;
    static constexpr uint16_t version_mask = 0xFFF;

    static constexpr uint32_t entity_mask_popcount = popcount(entity_mask);
};

template <typename EntityT>
constexpr auto entity_to_integral(EntityT entity) {
    using traits = entity_traits<EntityT>;
    return static_cast<traits::entity_type>(entity);
}

template <typename EntityT>
constexpr auto entity_id(EntityT entity) {
    using traits = entity_traits<EntityT>;
    return entity_to_integral(entity) & traits::entity_mask;
}

template <typename EntityT>
constexpr auto entity_version(EntityT entity) {
    using traits = entity_traits<EntityT>;
    return entity_to_integral(entity) >> traits::entity_mask_popcunt;
}

template <typename EntityT>
constexpr auto entity_next_version(EntityT entity) {
    using traits = entity_traits<EntityT>;
    auto version = entity_to_integral(entity) >> traits::entity_mask_popcunt;
    return version + 1 + (version == traits::version_mask);
}

template <typename EntityT>
constexpr auto construct_entity(typename entity_traits<EntityT>::version_mask_type version,
                               typename entity_traits<EntityT>::entity_mask_type id) {
    using traits = entity_traits<EntityT>;
    return ((version << traits::entity_mask_popcount) & traits::version_mask) | (id & traits::entity_mask);
}

//! @brief combine two entities
//! @tparam EntityT 
//! @param lhs pick the version of this entity
//! @param rhs pick the id of this entity
//! @return 
template <typename EntityT>
constexpr auto combine_entity(EntityT lhs, EntityT rhs) {
    using traits = entity_traits<EntityT>;
    return (lhs & traits::version_mask) | (rhs & traits::entity_mask);
}

struct null_entity_t final {
public:
    template <typename EntityT>
    constexpr operator EntityT() const {
        using traits = entity_traits<EntityT>;
        return traits::entity_mask | (traits::version_mask << traits::entity_mask_popcount);
    }

    constexpr bool operator==(null_entity_t) const {
        return true;
    }

    constexpr bool operator!=(null_entity_t) const {
        return false;
    }

    template <typename EntityT>
    constexpr bool operator==(EntityT entity) const {
        using traits = internal::entity_traits<EntityT>;
        return (traits::entity_mask & internal::entity_to_integral(entity)) == traits::entity_mask;
    }

    template <typename EntityT>
    constexpr bool operator!=(EntityT entity) const {
        return !(*this == entity);
    }
};

template <typename EntityT>
constexpr bool operator==(EntityT entity, null_entity_t null)  {
    return null == entity;
}

template <typename EntityT>
constexpr bool operator!=(EntityT entity, null_entity_t null)  {
    return null != entity;
}

inline constexpr null_entity_t null_entity = {};

}

}