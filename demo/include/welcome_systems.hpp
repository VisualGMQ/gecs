#pragma once

#include "defs.hpp"
#include "game_ctx.hpp"

void OnEnterWelcome(gecs::commands cmds, gecs::resource<TextureManager> img_mgr, gecs::event_dispatcher<SDL_KeyboardEvent> keyboard);
void OnExitWelcome(gecs::event_dispatcher<SDL_KeyboardEvent> keyboard);