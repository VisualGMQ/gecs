#pragma once

#include "world.hpp"
#include "gecs/config/config.hpp"

namespace gecs {

using world = basic_world<config::Entity, config::PageSize>;

}