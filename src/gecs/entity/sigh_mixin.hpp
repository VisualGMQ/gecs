#pragma once

#include "gecs/signal/sigh.hpp"
#include "storage.hpp"

namespace gecs {

/**
 * @brief make basic_storage support signal system
 * 
 * @tparam T  basic_storage<>
 */
template <typename T>
class sigh_mixin final : public T {
public:
    using underlying_type = T;
    using entity_type = typename underlying_type::entity_type;
    using payload_type = typename underlying_type::payload_type;
    using sigh_type = sigh<void(entity_type, payload_type& payload)>;

    //! @brief get the construction signals
    auto& on_construct() noexcept {
        return this->construction_;
    }

    //! @brief get the update signals
    auto& on_update() noexcept {
        return this->update_;
    }

    //! @brief get the destruction signals
    auto& on_destruction() noexcept {
        return this->destruction_;
    }

    template <typename... Args>
    payload_type& emplace(entity_type entity, Args&&... args) noexcept {
        auto& payload = underlying_type::emplace(entity, std::forward<Args>(args)...);
        construction_.trigger(entity, payload);
        return payload;
    }

    template <typename... Args>
    payload_type& replace(entity_type entity, Args&&... args) noexcept {
        auto& payload = underlying_type::replace(entity, std::forward<Args>(args)...);
        update_.trigger(entity, payload);
        return payload;
    }

    void remove(entity_type entity) noexcept {
        destruction_.trigger(entity, underlying_type::operator[](entity));
        underlying_type::remove(entity);
    }

private:
    sigh_type construction_;
    sigh_type update_;
    sigh_type destruction_;
};

template <typename EntityT, size_t PageSize>
class sigh_mixin<basic_storage<EntityT, EntityT, PageSize, void>> final : public basic_storage<EntityT, EntityT, PageSize, void> {
public:
    using underlying_type = basic_storage<EntityT, EntityT, PageSize, void>;
    using entity_type = typename underlying_type::entity_type;
    using sigh_type = sigh<void(entity_type)>;

    auto& on_construct() noexcept {
        return this->construction_;
    }

    auto& on_destruction() noexcept {
        return this->destruction_;
    }

    entity_type emplace() noexcept {
        entity_type entity = underlying_type::emplace();
        construction_.trigger(entity);
        return entity;
    }

    void remove(entity_type entity) noexcept {
        destruction_.trigger(entity);
        underlying_type::remove(entity);
    }

private:
    sigh_type construction_;
    sigh_type destruction_;
};

}