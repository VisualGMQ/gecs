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

// a tag for bullet entity
struct bullet {};
// a tag for player's tank
struct Tank {};

gecs::entity create_bullet(const Vector2& init_pos, const Vector2& vel, gecs::commands cmds, gecs::resource<gecs::mut<TextureManager>> ts_mgr) {
    auto entity = cmds.create();
    auto ts = ts_mgr->FindTilesheet("shell");
    auto anim = animation()
                    .add(ts->Get(0, 0), 3)
                    .add(ts->Get(1, 0), 3)
                    .set_loop(-1)
                    .play();
    cmds.emplace<animation>(entity, anim);
    cmds.emplace<sprite>(entity, Image{}, init_pos, DepthLayer::BulletDepth);
    cmds.emplace<bullet>(entity);
    cmds.emplace<rigidbody>(entity, rigidbody{vel, Rect{}});
    return entity;
}

// startup system to init SDL and 
void startup(gecs::commands cmds, gecs::event_dispatcher<SDL_QuitEvent> quit) {
    SDL_Init(SDL_INIT_EVERYTHING);

    auto& ctx = cmds.emplace_resource<game_context>(game_context::create("demo", 1024, 720));
    ctx.renderer->SetScale(Vector2{ScaleFactor});

    constexpr auto f = +[](const SDL_QuitEvent& event, gecs::world& world){
        world.res<gecs::mut<game_context>>()->shouldClose = true;
    };
    quit.sink().add<f>();

    auto& ts_mgr = cmds.emplace_resource<TextureManager>(ctx.renderer.get());
    auto& tank_ts = ts_mgr.LoadTilesheet("tank", "demo/resources/tank.bmp", KeyColor, 2, 1);
    ts_mgr.LoadTilesheet("shell", "demo/resources/shell.bmp", KeyColor, 2, 1);

    auto tank_entity = cmds.create();

    auto tank_anim = animation()
                        .add(tank_ts.Get(0, 0), 3)
                        .add(tank_ts.Get(1, 0), 3)
                        .set_loop(-1).play();
    cmds.emplace<animation>(tank_entity, tank_anim);
    cmds.emplace<rigidbody>(tank_entity, rigidbody{Vector2{0, 0}, Rect{}});
    cmds.emplace<sprite>(tank_entity, tank_ts.Get(0, 0), Vector2{CanvaSize.w / 2.0f, TankY}, DepthLayer::TankDepth);
    cmds.emplace<Tank>(tank_entity);
    cmds.emplace<ticker>(tank_entity, 10);

    auto land_entity = cmds.create();
    cmds.emplace<sprite>(land_entity, Image(ts_mgr.Load("land", "demo/resources/land.bmp", KeyColor)), Vector2{0.0f, CanvaSize.h - 100.0f}, DepthLayer::LandDepth);
    cmds.emplace<rigidbody>(land_entity, rigidbody{Vector2{0, 0}, Rect{}});
}

void update_anim(gecs::querier<gecs::mut<animation>> anims) {
    for (auto& [_, anim] : anims) {
        anim.update();
    }
}

void update_rigidbody(gecs::querier<gecs::mut<sprite>, rigidbody> bodies) {
    for (auto& [entity, sprite, body] : bodies) {
        sprite.position += body.velocity;
    }
}

void play_anim_by_vel(gecs::querier<Tank, gecs::mut<animation>, rigidbody> querier) {
    for (auto& [_, tank, anim, body] : querier) {
        if (body.velocity.x == 0) {
            anim.stop();
        } else {
            anim.play();
        }
    }
}

void update_anim_to_image(gecs::querier<animation, gecs::mut<sprite>> querier) {
    for (auto& [_, anim, sprite] : querier) {
        if (anim.is_playing()) {
            sprite.image = anim.cur_image();
        }
    }
}

void render_image(gecs::querier<sprite> querier, gecs::resource<game_context> ctx) {
    for (auto& [_, sprite] : querier.sort_by<sprite>([](const sprite& s1, const sprite& s2){
        return s1.depth < s2.depth;
    })) {
        ctx->renderer->DrawImage(sprite.image, sprite.position, std::nullopt);
    }
}

// shutdown system to clear all resources and close SDL
void shutdown(gecs::commands cmds) {
    cmds.remove_resource<TextureManager>();
    SDL_Quit();
}

// dispatch `SDL_QuitEvent` to quit game
void event_dispatcher(gecs::resource<gecs::mut<game_context>> ctx, gecs::event_dispatcher<SDL_QuitEvent> quit) {
    while (SDL_PollEvent(&ctx->event)) {
        if (ctx->event.type == SDL_QUIT) {
            quit.enqueue(ctx->event.quit);
        }
    }
}

// clear screen and render present
void render_update(gecs::resource<game_context> ctx) {
    ctx->renderer->Present();
    ctx->renderer->SetColor({50, 50, 50, 255});
    ctx->renderer->Clear();
    SDL_Delay(30);
}

void move_tank(gecs::querier<Tank, gecs::mut<sprite>, gecs::mut<rigidbody>> querier) {
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

void shoot_bullet(
    gecs::commands cmds, gecs::resource<gecs::mut<TextureManager>> res,
    gecs::querier<Tank, sprite, gecs::mut<ticker>> querier, gecs::resource<gecs::mut<game_context>> ctx) {
    for (auto& [_, tank, sprite, ticker] : querier) {
        if (ticker.is_end() && SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_J]) {
            auto entity = create_bullet(sprite.position - Vector2(0, 16), Vector2{0, -5}, cmds, res);

            ticker.reset();
        }
    }
}

void update_ticker(gecs::querier<gecs::mut<ticker>> tickers) {
    for (auto& [_, ticker] : tickers) {
        ticker.update();
    }
}

void remove_bullet(gecs::commands cmds, gecs::querier<bullet, sprite> querier, gecs::resource<gecs::mut<game_context>> ctx) {
    for (auto& [entity, bullet, sprite] : querier) {
        if (sprite.position.y + 32 < 0 || sprite.position.y >= CanvaSize.h) {
            cmds.destroy(entity);
        }
    }
}

int main(int argc, char** argv) {
    gecs::world world;

    // regist all systems
    // startup system
    world.regist_startup_system<startup>()
    // shutdown system
        .regist_shutdown_system<shutdown>()
    // update system
        .regist_update_system<event_dispatcher>()
        .regist_update_system<move_tank>()
        .regist_update_system<shoot_bullet>()
        .regist_update_system<play_anim_by_vel>()
        .regist_update_system<update_anim>()
        .regist_update_system<update_anim_to_image>()
        .regist_update_system<update_rigidbody>()
        .regist_update_system<update_ticker>()
        .regist_update_system<render_image>()
        .regist_update_system<render_update>()
        .regist_update_system<remove_bullet>();

    // startup ecs
    world.startup();

    // use world to access game_context directly
    gecs::resource<game_context> res = world.res<game_context>();
    while (!res->shouldClose) {
        // update ecs
        world.update();
    }

    // shutdown ecs
    world.shutdown();
    return 0;
}
