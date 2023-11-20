#include "restart_systems.hpp"

constexpr auto change_state = +[](const SDL_KeyboardEvent& key, gecs::commands cmds) {
    if (key.type == SDL_KEYDOWN && key.keysym.scancode == SDL_SCANCODE_R) {
        cmds.switch_state(GameState::Gaming);
    }
};

void OnEnterRestart(gecs::commands cmds, gecs::resource<TextureManager> ts_mgr, gecs::event_dispatcher<SDL_KeyboardEvent> keyboard) {
    auto entity = cmds.create();
    cmds.emplace<Sprite>(entity, *(ts_mgr->Find("gameover")), Vector2{170, 170}, DepthLayer::UIDepth);
    cmds.emplace<RestartScene>(entity);

    keyboard.sink().add<change_state>();
}

void OnExitRestart(gecs::commands cmds, gecs::querier<RestartScene> querier, gecs::event_dispatcher<SDL_KeyboardEvent> keyboard) {
    for (auto&& [ent, _] : querier) {
        cmds.destroy(ent);
    }
    keyboard.sink().remove<change_state>();
}
