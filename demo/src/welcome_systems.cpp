#include "welcome_systems.hpp"

constexpr auto to_gaming_state = +[](const SDL_KeyboardEvent& keyboard, gecs::commands cmds, gecs::querier<WelcomeScene> querier) {
    if (keyboard.type == SDL_KEYDOWN) {
        for (auto& [ent, _] : querier) {
            cmds.destroy(ent);
        }

        cmds.switch_state(GameState::Gaming);
    }
};

void OnEnterWelcome(gecs::commands cmds, gecs::resource<TextureManager> img_mgr, gecs::event_dispatcher<SDL_KeyboardEvent> keyboard) {
    auto how_to_play_ent = cmds.create();
    cmds.emplace<WelcomeScene>(how_to_play_ent);
    cmds.emplace<Sprite>(how_to_play_ent, Image(*(img_mgr->Find("how_to_play"))), Vector2(140, 100), WelcomeDepth);

    auto how_to_start_ent = cmds.create();
    cmds.emplace<WelcomeScene>(how_to_start_ent);
    cmds.emplace<Sprite>(how_to_start_ent, Image(*(img_mgr->Find("how_to_start"))), Vector2(40, 270), WelcomeDepth);
    
    keyboard.sink().add<to_gaming_state>();
}

void OnExitWelcome(gecs::event_dispatcher<SDL_KeyboardEvent> keyboard) {
    keyboard.sink().clear();
}