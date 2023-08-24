#pragma once

#include "defs.hpp"
#include "game_ctx.hpp"

void UpdateAnim(gecs::querier<gecs::mut<Animation>> anims);
void UpdateRigidbody(gecs::querier<gecs::mut<Sprite>, RigidBody> bodies);
void UpdateAnimToImage(gecs::querier<Animation, gecs::mut<Sprite>> querier);
void RenderSprite(gecs::querier<Sprite> querier,
                  gecs::resource<GameContext> ctx);
void UpdateTicker(gecs::querier<gecs::mut<Ticker>> tickers);