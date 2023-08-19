#pragma once

#include "world.hpp"
#include "gecs/config/config.hpp"
#include "gecs/entity/querier.hpp"
#include "gecs/entity/resource.hpp"
#include "gecs/entity/commands.hpp"

namespace gecs {

using world = basic_world<config::Entity, config::PageSize>;

template <typename... Ts>
using querier = typename world::querier_type<Ts...>;

using commands = typename world::commands_type;

template <typename T>
using event_dispatcher = typename world::event_dispatcher_type<T>;

}