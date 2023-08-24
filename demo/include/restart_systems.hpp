#pragma once

#include "defs.hpp"
#include "gecs/gecs.hpp"

void OnEnterRestart(gecs::commands cmds, gecs::resource<TextureManager> ts_mgr, gecs::event_dispatcher<SDL_KeyboardEvent> keyboard);
void OnExitRestart(gecs::commands cmds, gecs::querier<RestartScene> querier, gecs::event_dispatcher<SDL_KeyboardEvent> keyboard);