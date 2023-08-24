#include "common_systems.hpp"

void UpdateAnim(gecs::querier<gecs::mut<Animation>> anims) {
    for (auto& [_, anim] : anims) {
        anim.Update();
    }
}

void UpdateRigidbody(gecs::querier<gecs::mut<Sprite>, RigidBody> bodies) {
    for (auto& [entity, sprite, body] : bodies) {
        sprite.position += body.velocity;
    }
}

void UpdateAnimToImage(gecs::querier<Animation, gecs::mut<Sprite>> querier) {
    for (auto& [_, anim, sprite] : querier) {
        if (anim.IsPlaying()) {
            sprite.image = anim.CurImage();
        }
    }
}

void RenderSprite(gecs::querier<Sprite> querier,
                  gecs::resource<GameContext> ctx) {
    for (auto& [_, sprite] :
         querier.sort_by<Sprite>([](const Sprite& s1, const Sprite& s2) {
             return s1.depth < s2.depth;
         })) {
        ctx->renderer->DrawImage(sprite.image, sprite.position, std::nullopt);
    }
}

void UpdateTicker(gecs::querier<gecs::mut<Ticker>> tickers) {
    for (auto& [_, ticker] : tickers) {
        ticker.Update();
    }
}