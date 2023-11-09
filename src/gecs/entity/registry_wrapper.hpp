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

    GECS_REFERENCE_ANY get_mut(entity_type entity, const config::type_info& type_info) noexcept {
        return reg_->get_mut(entity, type_info);
    }

    template <typename Type>
    const Type& get(entity_type entity) const noexcept {
        return reg_->template get<Type>(entity);
    }

    template <typename Type>
    Type& get_mut(entity_type entity) noexcept {
        return reg_->template get_mut<Type>(entity);
    }

    auto& entities() const noexcept { return reg_->entities(); }

    template <typename T>
    auto res() noexcept {
        return reg_->template res<T>();
    }

private:
    registry_type* reg_;
};

}  // namespace gecs