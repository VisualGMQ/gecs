#pragma once

#include "gecs/gecs.hpp"
#include "defs.hpp"
#include "game_ctx.hpp"

#include <random>

// a class for score
struct Score { int order; };

void RemoveBullet(gecs::commands cmds, gecs::querier<Bullet, Sprite> querier,
                  gecs::resource<gecs::mut<GameContext>> ctx);

void RenderCollideBox(gecs::resource<gecs::mut<GameContext>> ctx,
                      gecs::querier<Sprite, RigidBody> querier);

void RemoveFinishedBombAnim(gecs::commands cmds,
                            gecs::querier<Bomb, Animation> querier);

void FallingStoneGenerate(gecs::commands cmds,
                          gecs::resource<AnimManager> anim_mgr,
                          gecs::resource<gecs::mut<GameContext>> ctx);

void MoveTank(
    gecs::querier<Tank, gecs::mut<Sprite>, gecs::mut<RigidBody>> querier);

void ShootBullet(gecs::commands cmds, gecs::resource<AnimManager> anim_mgr,
                 gecs::querier<Tank, Sprite, gecs::mut<Ticker>> querier,
                 gecs::resource<gecs::mut<GameContext>> ctx);

void PlayAnimByVel(
    gecs::querier<Tank, gecs::mut<Animation>, RigidBody> querier);

void OnEnterGaming(gecs::commands cmds, gecs::resource<AnimManager> anim_mgr, gecs::resource<TextureManager> ts_mgr, gecs::resource<gecs::mut<GameContext>> ctx);
void UpdateScoreImage(gecs::resource<GameContext> ctx,
                      gecs::querier<Score, gecs::mut<Sprite>> scores,
                      gecs::resource<TextureManager> ts_mgr);

void OnExitGaming(gecs::commands cmds, gecs::querier<GamingScene> querier);

template <typename T1, typename T2>
void CollideHandle(gecs::commands cmds,
                   gecs::querier<T1, RigidBody, Sprite> querier1,
                   gecs::querier<T2, RigidBody, Sprite> querier2,
                   gecs::resource<AnimManager> anim_mgr,
                   gecs::resource<gecs::mut<GameContext>> ctx) {
    float min = std::numeric_limits<float>::max();
    std::vector<std::pair<Vector2, gecs::entity>> entities1;
    std::vector<gecs::entity> entities2;

    for (auto&& [ent1, _, body1, sprite1] : querier1) {
        gecs::entity entity = gecs::null_entity;
        Rect entity1_rect(body1.collide.position + sprite1.position,
                          body1.collide.size);

        for (auto&& [ent2, _, body2, sprite2] : querier2) {
            if (!cmds.alive(ent2)) {
                break;
            }
            Rect entity2_rect(body2.collide.position + sprite2.position,
                              body2.collide.size);
            if (IsRectIntersect(entity2_rect, entity1_rect)) {
                float t =
                    RectCollidTime(entity1_rect, entity2_rect, body1.velocity);
                if (t < min) {
                    entity = ent2;
                }
            }
        }

        if (entity != gecs::null_entity) {
            entities1.push_back({
                {entity1_rect.x - 7, entity1_rect.y - 34},
                ent1
            });
            entities2.push_back(entity);
        }
    }

    bool is_game_over = false;

    for (auto&& [pos, ent] : entities1) {
        cmds.destroy(ent);
        auto& body = CreateBombAnim(pos, FallingStoneVel, cmds, anim_mgr);
        if constexpr (std::is_same_v<T2, Land>) {
            body.velocity.Set(0, 0);
        }
        if constexpr (std::is_same_v<T2, Bullet>) {
            ctx->score ++;
        }
        if constexpr (std::is_same_v<T2, Tank>) {
            is_game_over = true;
        }
    }

    if constexpr (!std::is_same_v<T2, Land>) {
        for (auto ent : entities2) {
            cmds.destroy(ent);
        }
    }

    if (is_game_over) {
        cmds.switch_state(GameState::Restart);
    }
}