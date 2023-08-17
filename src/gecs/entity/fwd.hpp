#pragma once

#include "world.hpp"
#include "gecs/config/config.hpp"

namespace gecs {

using world = basic_world<config::Entity, config::PageSize>;

template <typename... Ts>
using querier = typename world::querier_type<Ts...>;

using commands = typename world::commands_type;

}