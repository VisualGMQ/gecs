#include "game_ctx.hpp"
#include "anim.hpp"
#include "physics.hpp"
#include "consts.hpp"
#include "ticker.hpp"
#include <iostream>

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
    cmds.emplace<Image>(entity);
    cmds.emplace<bullet>(entity);
    cmds.emplace<rigidbody>(entity, rigidbody{vel, init_pos, Rect{}});
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
    cmds.emplace<rigidbody>(tank_entity, rigidbody{Vector2{0, 0}, Vector2{CanvaSize.w / 2.0f, TankY}, Rect{}});
    cmds.emplace<Image>(tank_entity, tank_ts.Get(0, 0));
    cmds.emplace<Tank>(tank_entity);
    cmds.emplace<ticker>(tank_entity, 10);

    auto land_entity = cmds.create();
    auto img = cmds.emplace<Image>(land_entity, Image(ts_mgr.Load("land", "demo/resources/land.bmp", KeyColor)));
    cmds.emplace<rigidbody>(land_entity, rigidbody{Vector2{0, 0}, Vector2{0.0f, CanvaSize.h - 100.0f}, Rect{}});
}

void update_anim(gecs::querier<gecs::mut<animation>> anims) {
    for (auto& [_, anim] : anims) {
        anim.update();
    }
}

void update_rigidbody(gecs::querier<gecs::mut<rigidbody>> bodies) {
    for (auto& [entity, body] : bodies) {
        body.position += body.velocity;
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

void update_anim_to_image(gecs::querier<animation, gecs::mut<Image>> querier) {
    for (auto& [_, anim, image] : querier) {
        if (anim.is_playing()) {
            int frame = anim.cur_frame_idx();
            auto img = anim.cur_image();
            image = img;
        }
    }
}

void render_image(gecs::querier<Image, rigidbody> querier, gecs::resource<game_context> ctx) {
    for (auto& [_, image, rigidbody] : querier) {
        ctx->renderer->DrawImage(image, rigidbody.position, std::nullopt);
    }
}

// shutdown system to clear all resources and close SDL
void shutdown(gecs::commands cmds) {
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

void move_tank(gecs::querier<Tank, gecs::mut<rigidbody>> querier) {
    for (auto& [_, tank, rigidbody] : querier) {
        if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_A]) {
            rigidbody.velocity = Vector2{-TankSpeed, 0};
        } else if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_D]) {
            rigidbody.velocity = Vector2{TankSpeed, 0};
        } else {
            rigidbody.velocity.Set(0, 0);
        }
        rigidbody.position.x = std::clamp<float>(rigidbody.position.x, 0, CanvaSize.w - 32);
    }
}

void shoot_bullet(
    gecs::commands cmds, gecs::resource<gecs::mut<TextureManager>> res,
    gecs::querier<Tank, rigidbody, gecs::mut<ticker>> querier, gecs::resource<gecs::mut<game_context>> ctx) {
    for (auto& [_, tank, body, ticker] : querier) {
        if (ticker.is_end() && SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_J]) {
            auto entity = create_bullet(body.position - Vector2(0, 16), Vector2{0, -5}, cmds, res);

            ticker.reset();
        }
    }
}

void update_ticker(gecs::querier<gecs::mut<ticker>> tickers) {
    for (auto& [_, ticker] : tickers) {
        ticker.update();
    }
}

void remove_bullet(gecs::commands cmds, gecs::querier<bullet, rigidbody> querier, gecs::resource<gecs::mut<game_context>> ctx) {
    // TODO: add tombstone into ecs to support eazier erase
    std::vector<bool> should_remove(querier.size(), false);

    size_t i = should_remove.size() - 1;
    for (auto& [entity, bullet, body] : querier) {
        if (body.position.y + 32 < 0 || body.position.y >= CanvaSize.h) {
            should_remove[i] = true;
        }
        i--;
    }

    for (int i = 0; i < should_remove.size(); i++) {
        if (should_remove[i]) {
            auto& entity = querier.entities()[i];
            cmds.destroy(static_cast<gecs::entity>(entity));
        }
    }
}

int main(int argc, char** argv) {
    gecs::world world;

    // regist all systems
    world.regist_startup_system<startup>();
    world.regist_shutdown_system<shutdown>();

    world.regist_update_system<event_dispatcher>();
    world.regist_update_system<move_tank>();
    world.regist_update_system<shoot_bullet>();
    world.regist_update_system<play_anim_by_vel>();
    world.regist_update_system<update_anim>();
    world.regist_update_system<update_anim_to_image>();
    world.regist_update_system<update_rigidbody>();
    world.regist_update_system<update_ticker>();
    world.regist_update_system<render_image>();
    world.regist_update_system<render_update>();
    world.regist_update_system<remove_bullet>();

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
