#pragma once

#include "gecs/config/config.hpp"

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
        static value_type value = curr_++;
        return value;
    }

private:
    inline static value_type curr_ = {};
};

using component_id_generator = basic_id_generator<internal::component_tag>;
using dispatcher_id_generator =
    basic_id_generator<internal::event_dispatcher_tag>;

}  // namespace gecs