#pragma once

#include "registry.hpp"
#include <map>

namespace gecs {

template <typename EntityT, size_t PageSize>
class basic_world final {
public:
    using self_type = basic_world<EntityT, PageSize>;
    using registry_type = basic_registry<self_type, EntityT, PageSize>;
    using registry_container = std::map<std::string, registry_type>;
    using registry_container_iterator = typename registry_container::iterator;

    basic_world() {
        cur_registry_ = registries_.end();
        will_switch_registry_ = registries_.end();
    }

    registry_type& regist_registry(const std::string& name) noexcept {
        return registries_.emplace(name, *this).first->second;
    }

    void start_with(const std::string& name) noexcept {
        if (auto it = registries_.find(name); it != registries_.end()) {
            cur_registry_ = it;
        }
    }

    template <typename T>
    auto res() const {
        return resource<T>{};
    }

    template <typename T>
    auto res_mut() const {
        return resource<gecs::mut<T>>{};
    }

    template <typename T>
    void remove_res() const {
        internal::resource_cache<T>::instance().remove();
    }

    void startup() noexcept {
        if (cur_registry_ != registries_.end()) {
            GECS_ASSERT(!registries_.empty(),
                        "you must create a scene before run app");
            cur_registry_ = registries_.begin();
        }

        cur_registry_->second.startup();
    }

    void update() noexcept {
        cur_registry_->second.update();

        if (will_switch_registry_ != registries_.end()) {
            do_switch_registry();
            will_switch_registry_ = registries_.end();
        }
    }

    void shutdown() noexcept { cur_registry_->second.shutdown(); }

    bool switch_registry(const std::string& name) noexcept {
        if (auto it = registries_.find(name); it == registries_.end()) {
            return false;
        } else {
            will_switch_registry_ = it;
            return true;
        }
    }

    void destroy_all_entities() {
        for (auto&& [name, reg] : registries_) {
            reg.destroy_all_entities();
        }
    }

    registry_type* cur_registry() noexcept { return &cur_registry_->second; }
    auto cur_registry() const noexcept { return &cur_registry_->second; }
    auto& cur_registry_name() const noexcept { return cur_registry_->first; }

private:
    registry_container registries_;
    registry_container_iterator cur_registry_;
    registry_container_iterator will_switch_registry_;

    void do_switch_registry() noexcept {
        if (cur_registry_ != registries_.end()) {
            cur_registry_->second.shutdown();
        }
        cur_registry_ = will_switch_registry_;
        cur_registry_->second.startup();
    }

    void do_switch_registry_no_shutdown() noexcept {
        will_switch_registry_->second.startup();
    }
};

}  // namespace gecs
