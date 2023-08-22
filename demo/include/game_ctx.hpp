#pragma once

#include "window.hpp"
#include "renderer.hpp"

struct BulletCreator final {
    gecs::entity operator()(gecs::commands cmds) const {
        auto entity = cmds.create();
    }
};

struct GameContext final {
    std::unique_ptr<Window> window;
    std::unique_ptr<Renderer> renderer;
    bool shouldClose;
    SDL_Event event;

    static GameContext Create(const std::string& title, int w, int h);
};