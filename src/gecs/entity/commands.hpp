#pragma once

#include "gecs/entity/entity.hpp"
#include "gecs/entity/resource.hpp"
#include <string>

namespace gecs {

/**
 * @brief a help class for create/delete/replace entity/resource/component from
 * basic_registry
 *
 * @tparam WorldT
 */
template <typename WorldT>
class basic_commands final {
public:
    using registry_type = typename WorldT::registry_type;
    using entity_type = typename registry_type::entity_type;

    basic_commands(WorldT& world, registry_type& reg) : world_(&world), reg_(&reg) {}

    entity_type create() noexcept { return reg_->create(); }

    template <typename Type, typename... Args>
    Type& emplace(entity_type entity, Args&&... args) noexcept {
        return reg_->template emplace<Type>(entity,
                                              std::forward<Args>(args)...);
    }

    template <typename Type, typename... Args>
    Type& replace(entity_type entity, Args&&... args) noexcept {
        return reg_->template replace<Type>(entity,
                                              std::forward<Args>(args)...);
    }

    template <typename T>
    void emplace_bundle(entity_type entity, T&& bundle) {
        reg_->template emplace_bundle(entity, std::forward<T>(bundle));
    }

    auto& destroy(entity_type entity) noexcept {
        reg_->destroy(entity);
        return *this;
    }

    template <typename Type>
    void remove(entity_type entity) noexcept {
        reg_->template remove<Type>(entity);
    }

    void remove(entity_type entity, const config::type_info& type) noexcept {
        reg_->remove(entity, type);
    }

    bool alive(entity_type entity) const noexcept {
        return reg_->alive(entity);
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
        reg_->template switch_state<T>(state);
        return *this;
    }

    template <typename T>
    auto& on_construct() noexcept {
        return reg_->template on_construct<T>();
    }

    template <typename T>
    auto& on_update() noexcept {
        return reg_->template on_update<T>();
    }

    template <typename T>
    auto& on_destruct() noexcept {
        return reg_->template on_destruct<T>();
    }

    auto& switch_registry(const std::string& name) noexcept {
        world_->switch_registry(name);
        return *this;
    }

private:
    WorldT* world_;
    registry_type* reg_;
};

}  // namespace gecs