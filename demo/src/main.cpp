#include "game_ctx.hpp"
#include "anim.hpp"
#include "physics.hpp"
#include "consts.hpp"
#include "ticker.hpp"
#include "sprite.hpp"

enum DepthLayer {
    LandDepth = 0,
    TankDepth,
    BulletDepth,
};

// a tag for falling stone entity
struct FallingStone {
    int hp;
};
// a tag for Bullet entity
struct Bullet {
    int damage;
};
// a tag for player's tank entity
struct Tank {
    int hp;
    int score;
};

gecs::entity CreateBullet(const Vector2& init_pos, const Vector2& vel, gecs::commands cmds, gecs::resource<AnimManager> anim_mgr) {
    auto entity = cmds.create();
    cmds.emplace<Animation>(entity, *(anim_mgr->Find("shell_fly")));
    cmds.emplace<Sprite>(entity, Image{}, init_pos, DepthLayer::BulletDepth);
    cmds.emplace<Bullet>(entity, Bullet{1});
    cmds.emplace<RigidBody>(entity, RigidBody{vel, Rect{15, 0, 15, 15}});
    return entity;
}

gecs::entity CreateFallingStone(const Vector2& init_pos, const Vector2& vel, gecs::commands cmds, gecs::resource<AnimManager> anim_mgr) {
    auto entity = cmds.create();
    cmds.emplace<Animation>(entity, *(anim_mgr->Find("shell_fly")));
    cmds.emplace<Sprite>(entity, Image{}, init_pos, DepthLayer::BulletDepth);
    cmds.emplace<FallingStone>(entity, FallingStone{1});
    cmds.emplace<RigidBody>(entity, RigidBody{vel, Rect{15, 0, 15, 15}});
    return entity;
}

// startup system to init SDL and resources
void Startup(gecs::commands cmds, gecs::event_dispatcher<SDL_QuitEvent> quit) {
    SDL_Init(SDL_INIT_EVERYTHING);

    auto& ctx = cmds.emplace_resource<GameContext>(GameContext::Create("demo", 1024, 720));
    ctx.renderer->SetScale(Vector2{ScaleFactor});

    auto& anim_mgr = cmds.emplace_resource<AnimManager>();

    constexpr auto f = +[](const SDL_QuitEvent& event, gecs::world& world){
        world.res<gecs::mut<GameContext>>()->shouldClose = true;
    };
    quit.sink().add<f>();

    auto& ts_mgr = cmds.emplace_resource<TextureManager>(ctx.renderer.get());
    auto& tank_ts = ts_mgr.LoadTilesheet("tank", "demo/resources/tank.bmp", KeyColor, 2, 1);
    auto& shell_ts = ts_mgr.LoadTilesheet("shell", "demo/resources/shell.bmp", KeyColor, 2, 1);
    anim_mgr.Create("shell_fly", {Frame(shell_ts.Get(0, 0), 3), Frame(shell_ts.Get(1, 0), 3)}).SetLoop(-1).Play();

    auto tank_entity = cmds.create();

    auto tank_anim = anim_mgr.Create("tank_move", {Frame(tank_ts.Get(0, 0), 3), Frame(tank_ts.Get(1, 0), 3)}).SetLoop(-1).Play();
    cmds.emplace<Animation>(tank_entity, tank_anim);
    cmds.emplace<RigidBody>(tank_entity, RigidBody{Vector2{0, 0}, Rect{0, 10, 32, 22}});
    cmds.emplace<Sprite>(tank_entity, tank_ts.Get(0, 0), Vector2{CanvaSize.w / 2.0f, TankY}, DepthLayer::TankDepth);
    cmds.emplace<Tank>(tank_entity, Tank{TankInitLife, 0});
    cmds.emplace<Ticker>(tank_entity, 10);

    auto land_entity = cmds.create();
    cmds.emplace<Sprite>(land_entity, Image(ts_mgr.Load("land", "demo/resources/land.bmp", KeyColor)), Vector2{0.0f, CanvaSize.h - 100.0f}, DepthLayer::LandDepth);
    cmds.emplace<RigidBody>(land_entity, RigidBody{Vector2{0, 0}, Rect{}});
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
void EventDispatcher(gecs::resource<gecs::mut<GameContext>> ctx, gecs::event_dispatcher<SDL_QuitEvent> quit) {
    while (SDL_PollEvent(&ctx->event)) {
        if (ctx->event.type == SDL_QUIT) {
            quit.enqueue(ctx->event.quit);
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
    for (auto& [_, sprite, body] : querier) {
        ctx->renderer->SetColor({0, 255, 0, 255});
        ctx->renderer->DrawRect(Rect{sprite.position + body.collide.position, body.collide.size});
    }
}

template <typename T1, typename T2>
void CollideHandle(gecs::commands cmds,
                  gecs::querier<T1, RigidBody, Sprite> querier1,
                  gecs::querier<T2, RigidBody, Sprite> querier2) {
    
    float min = std::numeric_limits<float>::max();
    for (auto& [ent1, _, body1, sprite1] : querier1) {
        gecs::entity entity = gecs::null_entity;

        for (auto& [ent2, _, body2, sprite2] : querier2) {
            Rect stone_rect(body2.collide.position + sprite2.position, body2.collide.size);
            Rect bullet_rect(body1.collide.position + sprite1.position, body1.collide.size);
            if (IsRectIntersect(stone_rect, bullet_rect)) {
                float t = RectCollidTime(bullet_rect, stone_rect, body1.velocity);
                if (t < min) {
                    entity = ent2;
                }
            }
        }

        if (entity != gecs::null_entity) {
            cmds.destroy(entity);
            cmds.destroy(ent1);
        }
    }
}

int main(int argc, char** argv) {
    gecs::world world;

    // regist all systems
    // startup system
    world.regist_startup_system<Startup>()
    // shutdown system
        .regist_shutdown_system<Shutdown>()
    // update system
        .regist_update_system<EventDispatcher>()
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
        .regist_update_system<CollideHandle<Bullet, FallingStone>>()
        .regist_update_system<CollideHandle<FallingStone, Tank>>()
        .regist_update_system<CollideHandle<Bullet, Tank>>()
        .regist_update_system<RenderUpdate>();

    // startup ecs
    world.startup();

    // use world to access GameContext directly
    gecs::resource<GameContext> res = world.res<GameContext>();
    while (!res->shouldClose) {
        // update ecs
        world.update();
    }

    // shutdown ecs
    world.shutdown();
    return 0;
}
