#include "gaming_systems.hpp"

void OnEnterGaming(gecs::commands cmds, gecs::resource<AnimManager> anim_mgr,
                   gecs::resource<TextureManager> ts_mgr,
                   gecs::resource<gecs::mut<GameContext>> ctx) {
    ctx->score = 0;

    auto tank_entity = cmds.create();
    cmds.emplace<GamingScene>(tank_entity);

    auto& tank_anim = *anim_mgr->Find("tank_move");
    cmds.emplace<Animation>(tank_entity, tank_anim);
    cmds.emplace<RigidBody>(tank_entity,
                            RigidBody{
                                Vector2{0, 0},
                                Rect{2, 10, 28, 22}
    });
    cmds.emplace<Sprite>(tank_entity, ts_mgr->FindTilesheet("tank")->Get(0, 0),
                         Vector2{CanvaSize.w / 2.0f, TankY},
                         DepthLayer::TankDepth);
    cmds.emplace<Tank>(tank_entity, Tank{TankInitLife, 0});
    cmds.emplace<Ticker>(tank_entity, 10);

    auto land_entity = cmds.create();
    cmds.emplace<GamingScene>(land_entity);
    cmds.emplace<Sprite>(
        land_entity,
        Image(*ts_mgr->Find("land")),
        Vector2{0.0f, CanvaSize.h - 100.0f}, DepthLayer::LandDepth);
    cmds.emplace<Land>(land_entity);
    cmds.emplace<RigidBody>(
        land_entity, RigidBody{
                         Vector2{0, 0},
                         Rect{0, 50, CanvaSize.w, CanvaSize.h}
    });

    auto score_entity1 = cmds.create();
    auto score_entity2 = cmds.create();
    cmds.emplace<GamingScene>(score_entity1);
    cmds.emplace<GamingScene>(score_entity2);
    uint32_t units = ctx->score % 10;
    uint32_t tens = ctx->score / 10;
    if (tens != 0) {
        cmds.emplace<Sprite>(score_entity1,
                             ts_mgr->FindTilesheet("numbers")->Get(tens, 0),
                             Vector2{10, 10}, DepthLayer::UIDepth);
    } else {
        cmds.emplace<Sprite>(score_entity1,
                             Image{},
                             Vector2{10, 10}, DepthLayer::UIDepth);
    }
    cmds.emplace<Score>(score_entity1, Score{0});
    cmds.emplace<Sprite>(score_entity2,
                            ts_mgr->FindTilesheet("numbers")->Get(units, 0),
                            Vector2{28, 10}, DepthLayer::UIDepth);
    cmds.emplace<Score>(score_entity2, Score{1});
}

void RemoveBullet(gecs::commands cmds, gecs::querier<Bullet, Sprite> querier,
                  gecs::resource<gecs::mut<GameContext>> ctx) {
    for (auto& [entity, bullet, sprite] : querier) {
        if (sprite.position.y + 32 < 0 || sprite.position.y >= CanvaSize.h) {
            cmds.destroy(entity);
        }
    }
}

void RenderCollideBox(gecs::resource<gecs::mut<GameContext>> ctx,
                      gecs::querier<Sprite, RigidBody> querier) {
    if (ctx->debugMode) {
        for (auto& [_, sprite, body] : querier) {
            ctx->renderer->SetColor({0, 255, 0, 255});
            ctx->renderer->DrawRect(Rect{
                sprite.position + body.collide.position, body.collide.size});
        }
    }
}

void RemoveFinishedBombAnim(gecs::commands cmds,
                            gecs::querier<Bomb, Animation> querier) {
    for (auto& [ent, _, anim] : querier) {
        if (anim.IsFinish()) {
            cmds.destroy(ent);
        }
    }
}


void FallingStoneGenerate(gecs::commands cmds,
                          gecs::resource<AnimManager> anim_mgr,
                          gecs::resource<gecs::mut<GameContext>> ctx) {
    ctx->stone_falling_ticker.Update();
    if (ctx->stone_falling_ticker.isFinish()) {
        float x = std::uniform_int_distribution<int>(
            50, CanvaSize.w - 50)(std::random_device());

        CreateFallingStone(Vector2(x, -100), FallingStoneVel, cmds, anim_mgr);
        ctx->stone_falling_ticker.Reset();
    }
}

void MoveTank(
    gecs::querier<Tank, gecs::mut<Sprite>, gecs::mut<RigidBody>> querier) {
    for (auto& [_, tank, sprite, rigidbody] : querier) {
        if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_A]) {
            rigidbody.velocity = Vector2{-TankSpeed, 0};
        } else if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_D]) {
            rigidbody.velocity = Vector2{TankSpeed, 0};
        } else {
            rigidbody.velocity.Set(0, 0);
        }
        sprite.position.x =
            std::clamp<float>(sprite.position.x, 0, CanvaSize.w - 32);
    }
}

void ShootBullet(gecs::commands cmds, gecs::resource<AnimManager> anim_mgr,
                 gecs::querier<Tank, Sprite, gecs::mut<Ticker>> querier,
                 gecs::resource<gecs::mut<GameContext>> ctx) {
    for (auto& [_, tank, sprite, ticker] : querier) {
        if (ticker.isFinish() &&
            SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_J]) {
            auto entity = CreateBullet(sprite.position - Vector2(0, 16),
                                       Vector2{0, -5}, cmds, anim_mgr);

            ticker.Reset();
        }
    }
}

void PlayAnimByVel(
    gecs::querier<Tank, gecs::mut<Animation>, RigidBody> querier) {
    for (auto& [_, tank, anim, body] : querier) {
        if (body.velocity.x == 0) {
            anim.Stop();
        } else {
            anim.Play();
        }
    }
}

void UpdateScoreImage(gecs::resource<GameContext> ctx,
                      gecs::querier<Score, gecs::mut<Sprite>> scores,
                      gecs::resource<TextureManager> ts_mgr) {
    scores.sort_by<Score>([](const Score& a, const Score& b){
        return a.order < b.order;
    });

    uint32_t units = ctx->score % 10;
    uint32_t tens = ctx->score / 10;
    auto it = scores.begin();
    {
        auto& [_, score, sprite] = *it;
        if (tens != 0) {
            sprite.image = ts_mgr->FindTilesheet("numbers")->Get(tens, 0);
        }
    }
    it ++;
    {
        auto& [_, score, sprite] = *it;
        sprite.image = ts_mgr->FindTilesheet("numbers")->Get(units, 0);
    }
}

void OnExitGaming(gecs::commands cmds, gecs::querier<GamingScene> querier) {
    for (auto& [ent, _] : querier) {
        cmds.destroy(ent);
    }
}
