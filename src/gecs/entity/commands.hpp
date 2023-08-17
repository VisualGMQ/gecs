#pragma once

#include "entity.hpp"
#include "resource.hpp"

namespace gecs {

template <typename WorldT>
class basic_commands final {
public:
    using entity_type = typename WorldT::entity_type; 

    basic_commands(WorldT* world): world_(world) {}

    entity_type create() noexcept {
        return world_->create();
    }

    template <typename Type, typename... Args>
    Type& emplace(entity_type entity, Args&&... args) noexcept {
        return world_->emplace<Type>(entity, std::forward<Args>(args)...);
    }

    template <typename Type, typename... Args>
    Type& replace(entity_type entity, Args&&... args) noexcept {
        return world_->replace<Type>(entity, std::forward<Args>(args)...);
    }

    void destroy(entity_type entity) noexcept {
        world_->destroy(entity);
    }

    template <typename Type>
    void remove(entity_type entity) noexcept {
        world_->remove<Type>(entity);
    }

    bool alive(entity_type entity) const noexcept {
        return world_->alive(entity);
    }

    template <typename T, typename... Args>
    T& emplace_resource(Args&&... args) noexcept {
        return internal::resource_cache<T>::instance().emplace(std::forward<Args>(args)...);
    }

    template <typename T>
    void remove_resource() noexcept {
        internal::resource_cache<T>::instance().remove();
    }

private:
    WorldT* world_;
};

}