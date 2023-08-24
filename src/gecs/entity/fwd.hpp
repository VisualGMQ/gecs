#pragma once

#include "gecs/config/config.hpp"
#include "gecs/entity/commands.hpp"
#include "gecs/entity/querier.hpp"
#include "gecs/entity/resource.hpp"
#include "world.hpp"

namespace gecs {

using world = basic_world<config::Entity, config::PageSize>;

template <typename... Ts>
using querier = typename world::querier_type<Ts...>;

using commands = typename world::commands_type;

template <typename T>
using event_dispatcher = typename world::event_dispatcher_type<T>;

}  // namespace gecs