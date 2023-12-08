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

    bool has(entity_type entity,
             const config::type_info& type_info) const noexcept {
        return reg_->has(entity, type_info);
    }

    GECS_ANY get_mut(entity_type entity,
                     const config::type_info& type_info) noexcept {
        return reg_->get_mut(entity, type_info);
    }

    template <typename Type>
    const Type& get(entity_type entity) const noexcept {
        return reg_->template get<Type>(entity);
    }

    auto commands() const noexcept { return reg_->commands(); }

    template <typename Type>
    Type& get_mut(entity_type entity) noexcept {
        return reg_->template get_mut<Type>(entity);
    }

    auto& entities() const noexcept { return reg_->entities(); }

    const auto& typeinfos() const noexcept { return reg_->typeinfos(); }

    const auto& pools() const noexcept { return reg_->pools(); }

    template <typename T>
    auto res() noexcept {
        return reg_->template res<T>();
    }

    template <auto System>
    void regist_startup_system(const std::string& name) {
        reg_->template regist_startup_system<System>(name);
    }

    template <auto System>
    void regist_update_system(const std::string& name) {
        reg_->template regist_update_system<System>(name);
    }

    template <auto System>
    void regist_shutdown_system(const std::string& name) {
        reg_->template regist_shutdown_system<System>(name);
    }

    template <auto System>
    void remove_startup_system(const std::string& name) {
        reg_->template remove_startup_system<System>(name);
    }

    template <auto System>
    void remove_update_system(const std::string& name) {
        reg_->template remove_update_system<System>(name);
    }

    template <auto System>
    void remove_shutdown_system(const std::string& name) {
        reg_->template remove_shutdown_system<System>(name);
    }

    template <auto System, typename T>
    auto& regist_enter_system_to_state(T value,
                                       const std::string& name) noexcept {
        reg_->template regist_enter_system_to_state<System>(value, name);
    }

    template <auto System, typename T>
    auto& regist_update_system_to_state(T value,
                                        const std::string& name) noexcept {
        reg_->template regist_update_system_to_state<System>(value, name);
    }

    template <auto System, typename T>
    auto& regist_exit_system_to_state(T value,
                                      const std::string& name) noexcept {
        reg_->template regist_exit_system_to_state<System>(value, name);
    }

    template <auto System, typename T>
    auto& remove_exit_system_to_state(T value,
                                      const std::string& name) noexcept {
        reg_->template remove_exit_system_to_state<System>(value, name);
    }

    template <auto System, typename T>
    auto& remove_update_system_to_state(T value,
                                        const std::string& name) noexcept {
        reg_->template remove_update_system_to_state<System>(value, name);
    }

    template <auto System, typename T>
    auto& remove_enter_system_to_state(T value,
                                       const std::string& name) noexcept {
        reg_->template remove_enter_system_to_state<System>(value, name);
    }

private:
    registry_type* reg_;
};

}  // namespace gecs