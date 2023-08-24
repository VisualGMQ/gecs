#pragma once

#include "renderer.hpp"
#include "ticker.hpp"
#include "window.hpp"

struct BulletCreator final {
    gecs::entity operator()(gecs::commands cmds) const {
        auto entity = cmds.create();
    }
};

struct GameContext final {
    std::unique_ptr<Window> window;
    std::unique_ptr<Renderer> renderer;
    bool shouldClose;
    Ticker stone_falling_ticker;
    SDL_Event event;
    bool debugMode = false;

    static GameContext Create(const std::string& title, int w, int h,
                              int falling_elapse);
};