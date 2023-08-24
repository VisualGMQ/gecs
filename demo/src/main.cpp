#include "defs.hpp"
#include "game_ctx.hpp"
#include "gaming_systems.hpp"
#include "common_systems.hpp"
#include "welcome_systems.hpp"

// startup system to init SDL and resources
void Startup(gecs::commands cmds, gecs::event_dispatcher<SDL_QuitEvent> quit,
             gecs::event_dispatcher<SDL_KeyboardEvent> keyboard) {
    SDL_Init(SDL_INIT_EVERYTHING);

    auto& ctx = cmds.emplace_resource<GameContext>(
        GameContext::Create("demo", 1024, 720, FallingStoneDuration));
    ctx.renderer->SetScale(Vector2{ScaleFactor});

    auto& anim_mgr = cmds.emplace_resource<AnimManager>();

    constexpr auto f = +[](const SDL_QuitEvent& event,
                           gecs::resource<gecs::mut<GameContext>> ctx) {
        ctx->shouldClose = true;
    };
    quit.sink().add<f>();

    constexpr auto f2 = +[](const SDL_KeyboardEvent& event,
                            gecs::resource<gecs::mut<GameContext>> ctx) {
        if (event.type == SDL_KEYDOWN &&
            event.keysym.scancode == SDL_SCANCODE_G) {
            ctx->debugMode = !ctx->debugMode;
        }
    };
    keyboard.sink().add<f2>();

    auto& ts_mgr = cmds.emplace_resource<TextureManager>(ctx.renderer.get());
    ts_mgr.Load("how_to_play", "demo/resources/how-to-play.bmp", KeyColor);
    ts_mgr.Load("land", "demo/resources/land.bmp", KeyColor);
    auto& tank_ts =
        ts_mgr.LoadTilesheet("tank", "demo/resources/tank.bmp", KeyColor, 2, 1);
    auto& shell_ts = ts_mgr.LoadTilesheet("shell", "demo/resources/shell.bmp",
                                          KeyColor, 2, 1);
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
        .Create("bomb",
                {Frame(bomb_ts.Get(0, 0), 2), Frame(bomb_ts.Get(1, 0), 2),
                 Frame(bomb_ts.Get(2, 0), 2), Frame(bomb_ts.Get(3, 0), 2),
                 Frame(bomb_ts.Get(4, 0), 2)})
        .SetLoop(-1)
        .Play();
    anim_mgr
        .Create("tank_move",
                {Frame(tank_ts.Get(0, 0), 3), Frame(tank_ts.Get(1, 0), 3)})
        .SetLoop(-1)
        .Play();
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

int main(int argc, char** argv) {
    gecs::world gaming_world;

    // regist all systems
    // startup system
    gaming_world
        .regist_startup_system<Startup>()
        // shutdown system
        .regist_shutdown_system<Shutdown>()
        // update systems
        // all state system
        .regist_update_system<EventDispatcher>()
        .regist_update_system<UpdateTicker>()
        .regist_update_system<UpdateAnim>()
        .regist_update_system<UpdateAnimToImage>()
        .regist_update_system<UpdateRigidbody>()
        .regist_update_system<RenderSprite>()
        .regist_update_system<RenderUpdate>()
        // welcome state
        .add_state(GameState::Welcome)
        .regist_enter_system_to_state<OnEnterWelcome>(GameState::Welcome)
        .regist_exit_system_to_state<OnExitWelcome>(GameState::Welcome)
        // gaming state
        .add_state(GameState::Gaming)
        .regist_enter_system_to_state<OnEnterGaming>(GameState::Gaming)
        .regist_update_system_to_state<FallingStoneGenerate>(GameState::Gaming)
        .regist_update_system_to_state<MoveTank>(GameState::Gaming)
        .regist_update_system_to_state<ShootBullet>(GameState::Gaming)
        .regist_update_system_to_state<PlayAnimByVel>(GameState::Gaming)
        .regist_update_system_to_state<RenderCollideBox>(GameState::Gaming)
        .regist_update_system_to_state<RemoveBullet>(GameState::Gaming)
        .regist_update_system_to_state<RemoveFinishedBombAnim>(
            GameState::Gaming)
        .regist_update_system_to_state<CollideHandle<FallingStone, Bullet>>(
            GameState::Gaming)
        .regist_update_system_to_state<CollideHandle<FallingStone, Tank>>(
            GameState::Gaming)
        .regist_update_system_to_state<CollideHandle<FallingStone, Land>>(
            GameState::Gaming)
        .start_with_state(GameState::Welcome);

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
