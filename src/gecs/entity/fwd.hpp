#pragma once

#include "gecs/config/config.hpp"
#include "gecs/entity/commands.hpp"
#include "gecs/entity/querier.hpp"
#include "gecs/entity/resource.hpp"
#include "gecs/entity/registry_wrapper.hpp"
#include "gecs/entity/world.hpp"

namespace gecs {

using world = basic_world<gecs::config::Entity, gecs::config::PageSize>;

template <typename... Ts>
using querier = typename world::registry_type::querier_type<Ts...>;

using commands = typename world::registry_type::commands_type;

template <typename T>
using event_dispatcher = typename world::registry_type::event_dispatcher_wrapper_type<T>;

using registry = registry_wrapper<typename world::registry_type>;

}  // namespace gecs
