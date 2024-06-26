#pragma once

#include "gecs/config/config.hpp"

namespace gecs {

/**
 * @brief a wrapper for basic_registry
 */
template <typename RegistryT>
class registry_wrapper final {
public:
    using registry_type = RegistryT;
    using entity_type = typename RegistryT::entity_type;

    registry_wrapper(registry_type& reg) : reg_(&reg) {}

    bool alive(entity_type entity) const noexcept {
        return reg_->alive(entity);
    }

    template <typename T>
    bool has(entity_type entity) const noexcept {
        return reg_->template has<T>(entity);
    }

    bool has(entity_type entity, const config::type_info& type_info) const noexcept {
        return reg_->has(entity, type_info);
    }

    GECS_ANY get_mut(entity_type entity, const config::type_info& type_info) noexcept {
        return reg_->get_mut(entity, type_info);
    }

    template <typename Type>
    const Type& get(entity_type entity) const noexcept {
        return reg_->template get<Type>(entity);
    }

    template <typename Type>
    const Type* try_get(entity_type entity) const noexcept {
        if (has<Type>(entity)) {
            return &reg_->template get<Type>(entity);
        } else {
            return nullptr;
        }
    }

    auto commands() const noexcept {
        return reg_->commands();
    }

    template <typename Type>
    Type& get_mut(entity_type entity) noexcept {
        return reg_->template get_mut<Type>(entity);
    }

    auto& entities() const noexcept { return reg_->entities(); }

    const auto& typeinfos() const noexcept {
        return reg_->typeinfos();
    }

    const auto& pools() const noexcept {
        return reg_->pools();
    }

    template <typename T>
    auto res() noexcept {
        return reg_->template res<T>();
    }

    template <typename T>
    auto res_mut() noexcept {
        return reg_->template res_mut<T>();
    }

    template <typename T>
    void call_system() {
        reg_->template call_system<T>();
    }

    template <typename T>
    auto construct_system() {
        return reg_->template construct_system<T>();
    }

    template <typename T>
    auto event_dispatcher() {
        return reg_->template event_dispatcher<T>();
    }

    template <typename T>
    auto cur_state() const {
        return reg_->template cur_state<T>();
    }

private:
    registry_type* reg_;
};

}  // namespace gecs