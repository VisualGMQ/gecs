#pragma once

#include "gecs/config/config.hpp"
#include <unordered_map>

namespace gecs {

namespace internal {
struct component_tag;
struct resource_tag;
struct event_dispatcher_tag;
}  // namespace internal

template <typename T>
struct basic_id_generator final {
public:
    using value_type = config::id_type;

    template <typename Type>
    static value_type gen() noexcept {
        value_type value = get_new_id<Type>();
        return value;
    }

    static value_type typeinfo_id(config::type_info typeinfo) noexcept {
        if (auto it = typeinfo_map_.find(typeinfo); it != typeinfo_map_.end()) {
            return it->second;
        } else {
            return typeinfo_map_.emplace(typeinfo, curr_++).second;
        }
    }

private:
    inline static value_type curr_ = {};
    inline static std::unordered_map<config::type_info, value_type>
        typeinfo_map_;

    template <typename U>
    static value_type get_new_id() noexcept {
        config::type_info type = GECS_GET_TYPE_INFO(U);
        if (auto it = typeinfo_map_.find(type); it != typeinfo_map_.end()) {
            return it->second;
        } else {
            auto id = curr_++;
            typeinfo_map_.emplace(type, id);
            return id;
        }
    }
};

using component_id_generator = basic_id_generator<internal::component_tag>;
using dispatcher_id_generator =
    basic_id_generator<internal::event_dispatcher_tag>;

}  // namespace gecs