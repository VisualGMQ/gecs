#include "game_ctx.hpp"
#include "defs.hpp"

#include <random>

// startup system to init SDL and resources
void Startup(gecs::commands cmds, gecs::event_dispatcher<SDL_QuitEvent> quit,
             gecs::event_dispatcher<SDL_KeyboardEvent> keyboard) {
  SDL_Init(SDL_INIT_EVERYTHING);

  auto& ctx = cmds.emplace_resource<GameContext>(
      GameContext::Create("demo", 1024, 720, FallingStoneDuration));
  ctx.renderer->SetScale(Vector2{ScaleFactor});

  auto& anim_mgr = cmds.emplace_resource<AnimManager>();

  constexpr auto f = +[](const SDL_QuitEvent &event, gecs::resource<gecs::mut<GameContext>> ctx) {
      ctx->shouldClose = true;
  };
  quit.sink().add<f>();

  constexpr auto f2 = +[](const SDL_KeyboardEvent& event, gecs::resource<gecs::mut<GameContext>> ctx) {
    if (event.type == SDL_KEYDOWN && event.keysym.scancode == SDL_SCANCODE_G) {
        ctx->debugMode = !ctx->debugMode;
    }
  };
  keyboard.sink().add<f2>();

  auto& ts_mgr = cmds.emplace_resource<TextureManager>(ctx.renderer.get());
  auto& tank_ts =
      ts_mgr.LoadTilesheet("tank", "demo/resources/tank.bmp", KeyColor, 2, 1);
  auto& shell_ts =
      ts_mgr.LoadTilesheet("shell", "demo/resources/shell.bmp", KeyColor, 2, 1);
  auto& falling_stone_ts = ts_mgr.LoadTilesheet(
      "falling_stone", "demo/resources/falling_stone.bmp", KeyColor, 2, 1);
  auto& bomb_ts =
      ts_mgr.LoadTilesheet("bomb", "demo/resources/bomb.bmp", KeyColor, 6, 1);
  anim_mgr
      .Create("shell_fly",
              {Frame(shell_ts.Get(0, 0), 3), Frame(shell_ts.Get(1, 0), 3)})
      .SetLoop(-1)
      .Play();
  anim_mgr
      .Create("falling_stone", {Frame(falling_stone_ts.Get(0, 0), 3),
                                Frame(falling_stone_ts.Get(1, 0), 3)})
      .SetLoop(-1)
      .Play();
  anim_mgr
      .Create("bomb", {Frame(bomb_ts.Get(0, 0), 2), Frame(bomb_ts.Get(1, 0), 2),
                       Frame(bomb_ts.Get(2, 0), 2), Frame(bomb_ts.Get(3, 0), 2),
                       Frame(bomb_ts.Get(4, 0), 2)})
      .SetLoop(-1)
      .Play();

  auto tank_entity = cmds.create();

  auto tank_anim = anim_mgr
                       .Create("tank_move", {Frame(tank_ts.Get(0, 0), 3),
                                             Frame(tank_ts.Get(1, 0), 3)})
                       .SetLoop(-1)
                       .Play();
  cmds.emplace<Animation>(tank_entity, tank_anim);
  cmds.emplace<RigidBody>(tank_entity,
                          RigidBody{Vector2{0, 0}, Rect{2, 10, 28, 22}});
  cmds.emplace<Sprite>(tank_entity, tank_ts.Get(0, 0),
                       Vector2{CanvaSize.w / 2.0f, TankY},
                       DepthLayer::TankDepth);
  cmds.emplace<Tank>(tank_entity, Tank{TankInitLife, 0});
  cmds.emplace<Ticker>(tank_entity, 10);

  auto land_entity = cmds.create();
  cmds.emplace<Sprite>(
      land_entity,
      Image(ts_mgr.Load("land", "demo/resources/land.bmp", KeyColor)),
      Vector2{0.0f, CanvaSize.h - 100.0f}, DepthLayer::LandDepth);
  cmds.emplace<Land>(land_entity);
  cmds.emplace<RigidBody>(
      land_entity,
      RigidBody{Vector2{0, 0}, Rect{0, 50, CanvaSize.w, CanvaSize.h}});
}

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

void PlayAnimByVel(gecs::querier<Tank, gecs::mut<Animation>, RigidBody> querier) {
    for (auto& [_, tank, anim, body] : querier) {
        if (body.velocity.x == 0) {
            anim.Stop();
        } else {
            anim.Play();
        }
    }
}

void UpdateAnimToImage(gecs::querier<Animation, gecs::mut<Sprite>> querier) {
    for (auto& [_, anim, sprite] : querier) {
        if (anim.IsPlaying()) {
            sprite.image = anim.CurImage();
        }
    }
}

void RenderSprite(gecs::querier<Sprite> querier, gecs::resource<GameContext> ctx) {
    for (auto& [_, sprite] : querier.sort_by<Sprite>([](const Sprite& s1, const Sprite& s2){
        return s1.depth < s2.depth;
    })) {
        ctx->renderer->DrawImage(sprite.image, sprite.position, std::nullopt);
    }
}

// shutdown system to clear all resources and close SDL
void Shutdown(gecs::commands cmds) {
    cmds.remove_resource<TextureManager>();
    SDL_Quit();
}

// dispatch `SDL_QuitEvent` to quit game
void EventDispatcher(gecs::resource<gecs::mut<GameContext>> ctx,
                     gecs::event_dispatcher<SDL_QuitEvent> quit,
                     gecs::event_dispatcher<SDL_KeyboardEvent> keyboard) {
    while (SDL_PollEvent(&ctx->event)) {
        if (ctx->event.type == SDL_QUIT) {
            quit.enqueue(ctx->event.quit);
        }
        if (ctx->event.type == SDL_KEYDOWN || ctx->event.type == SDL_KEYUP) {
            keyboard.enqueue(ctx->event.key);
        }
    }
}

// clear screen and render present
void RenderUpdate(gecs::resource<GameContext> ctx) {
    ctx->renderer->Present();
    ctx->renderer->SetColor({50, 50, 50, 255});
    ctx->renderer->Clear();
    SDL_Delay(30);
}

void MoveTank(gecs::querier<Tank, gecs::mut<Sprite>, gecs::mut<RigidBody>> querier) {
    for (auto& [_, tank, sprite, rigidbody] : querier) {
        if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_A]) {
            rigidbody.velocity = Vector2{-TankSpeed, 0};
        } else if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_D]) {
            rigidbody.velocity = Vector2{TankSpeed, 0};
        } else {
            rigidbody.velocity.Set(0, 0);
        }
        sprite.position.x = std::clamp<float>(sprite.position.x, 0, CanvaSize.w - 32);
    }
}

void ShootBullet(
    gecs::commands cmds, gecs::resource<AnimManager> anim_mgr,
    gecs::querier<Tank, Sprite, gecs::mut<Ticker>> querier, gecs::resource<gecs::mut<GameContext>> ctx) {
    for (auto& [_, tank, sprite, ticker] : querier) {
        if (ticker.isFinish() && SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_J]) {
            auto entity = CreateBullet(sprite.position - Vector2(0, 16), Vector2{0, -5}, cmds, anim_mgr);

            ticker.Reset();
        }
    }
}

void UpdateTicker(gecs::querier<gecs::mut<Ticker>> tickers) {
    for (auto& [_, ticker] : tickers) {
        ticker.Update();
    }
}

void RemoveBullet(gecs::commands cmds, gecs::querier<Bullet, Sprite> querier, gecs::resource<gecs::mut<GameContext>> ctx) {
    for (auto& [entity, bullet, sprite] : querier) {
        if (sprite.position.y + 32 < 0 || sprite.position.y >= CanvaSize.h) {
            cmds.destroy(entity);
        }
    }
}

void RenderCollideBox(gecs::resource<gecs::mut<GameContext>> ctx, gecs::querier<Sprite, RigidBody> querier) {
    if (ctx->debugMode) {
        for (auto& [_, sprite, body] : querier) {
            ctx->renderer->SetColor({0, 255, 0, 255});
            ctx->renderer->DrawRect(Rect{sprite.position + body.collide.position, body.collide.size});
        }
    }
}

void RemoveFinishedBombAnim(gecs::commands cmds, gecs::querier<Bomb, Animation> querier) {
    for (auto& [ent, _, anim] : querier) {
        if (anim.IsFinish()) {
            cmds.destroy(ent);
        }
    }
}

template <typename T1, typename T2>
void CollideHandle(gecs::commands cmds,
                  gecs::querier<T1, RigidBody, Sprite> querier1,
                  gecs::querier<T2, RigidBody, Sprite> querier2,
                  gecs::resource<AnimManager> anim_mgr) {
    float min = std::numeric_limits<float>::max();
    std::vector<std::pair<Vector2, gecs::entity>> entities1;
    std::vector<gecs::entity> entities2;

    for (auto& [ent1, _, body1, sprite1] : querier1) {
        gecs::entity entity = gecs::null_entity;
        Rect entity1_rect(body1.collide.position + sprite1.position, body1.collide.size);

        for (auto& [ent2, _, body2, sprite2] : querier2) {
            if (!cmds.alive(ent2)) {
                break;
            }
            Rect entity2_rect(body2.collide.position + sprite2.position, body2.collide.size);
            if (IsRectIntersect(entity2_rect, entity1_rect)) {
                float t = RectCollidTime(entity1_rect, entity2_rect, body1.velocity);
                if (t < min) {
                    entity = ent2;
                }
            }
        }

        if (entity != gecs::null_entity) {
            entities1.push_back({{entity1_rect.x - 7, entity1_rect.y - 34}, ent1});
            entities2.push_back(entity);
        }
    }

    for (auto [pos, ent] : entities1) {
        cmds.destroy(ent);
        auto& body = CreateBombAnim(pos, FallingStoneVel, cmds, anim_mgr);
        if constexpr(std::is_same_v<T2, Land>) {
            body.velocity.Set(0, 0);
        }
    }

    if constexpr (!std::is_same_v<T2, Land>) {
        for (auto ent : entities2) {
            cmds.destroy(ent);
        }
    }
}

void FallingStoneGenerate(gecs::commands cmds,
                          gecs::resource<AnimManager> anim_mgr,
                          gecs::resource<gecs::mut<GameContext>> ctx) {
    ctx->stone_falling_ticker.Update();
    if (ctx->stone_falling_ticker.isFinish()) {
        float x = std::uniform_int_distribution<int>(50, CanvaSize.w - 50)(std::random_device());

        CreateFallingStone(Vector2(x, -100), FallingStoneVel, cmds, anim_mgr);
        ctx->stone_falling_ticker.Reset();
    }
}

int main(int argc, char** argv) {
    gecs::world gaming_world;

    // regist all systems
    // startup system
    gaming_world.regist_startup_system<Startup>()
    // shutdown system
        .regist_shutdown_system<Shutdown>()
    // update system
        .regist_update_system<EventDispatcher>()
        .regist_update_system<FallingStoneGenerate>()
        .regist_update_system<MoveTank>()
        .regist_update_system<ShootBullet>()
        .regist_update_system<PlayAnimByVel>()
        .regist_update_system<UpdateAnim>()
        .regist_update_system<UpdateAnimToImage>()
        .regist_update_system<UpdateRigidbody>()
        .regist_update_system<UpdateTicker>()
        .regist_update_system<RenderSprite>()
        .regist_update_system<RenderCollideBox>()
        .regist_update_system<RemoveBullet>()
        .regist_update_system<RemoveFinishedBombAnim>()
        .regist_update_system<CollideHandle<FallingStone, Bullet>>()
        .regist_update_system<CollideHandle<FallingStone, Tank>>()
        .regist_update_system<CollideHandle<FallingStone, Land>>()
        .regist_update_system<RenderUpdate>();

    // startup ecs
    gaming_world.startup();

    // use world to access GameContext directly
    gecs::resource<GameContext> res = gaming_world.res<GameContext>();
    while (!res->shouldClose) {
        // update ecs
        gaming_world.update();
    }

    // shutdown ecs
    gaming_world.shutdown();
    return 0;
}
