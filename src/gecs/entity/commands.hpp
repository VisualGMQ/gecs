#pragma once

#include "entity.hpp"
#include "resource.hpp"

namespace gecs {

/**
 * @brief a help class for create/delete/replace entity/resource/component from
 * basic_world
 *
 * @tparam WorldT
 */
template <typename WorldT>
class basic_commands final {
public:
    using entity_type = typename WorldT::entity_type;

    basic_commands(WorldT& world) : world_(&world) {}

    entity_type create() noexcept { return world_->create(); }

    template <typename Type, typename... Args>
    Type& emplace(entity_type entity, Args&&... args) noexcept {
        return world_->template emplace<Type>(entity,
                                              std::forward<Args>(args)...);
    }

    template <typename Type, typename... Args>
    Type& replace(entity_type entity, Args&&... args) noexcept {
        return world_->template replace<Type>(entity,
                                              std::forward<Args>(args)...);
    }

    auto& destroy(entity_type entity) noexcept {
        world_->destroy(entity);
        return *this;
    }

    template <typename Type>
    void remove(entity_type entity) noexcept {
        world_->template remove<Type>(entity);
    }

    bool alive(entity_type entity) const noexcept {
        return world_->alive(entity);
    }

    template <typename T, typename... Args>
    T& emplace_resource(Args&&... args) noexcept {
        return internal::resource_cache<T>::instance().emplace(
            std::forward<Args>(args)...);
    }

    template <typename T>
    auto& remove_resource() noexcept {
        internal::resource_cache<T>::instance().remove();
        return *this;
    }

    template <typename T>
    auto& switch_state(T state) {
        world_->template switch_state<T>(state);
        return *this;
    }

    template <typename T>
    auto& on_construct() noexcept {
        return world_->template on_construct<T>();
    }

    template <typename T>
    auto& on_update() noexcept {
        return world_->template on_update<T>();
    }

    template <typename T>
    auto& on_destruct() noexcept {
        return world_->template on_destruct<T>();
    }

private:
    WorldT* world_;
};

}  // namespace gecs