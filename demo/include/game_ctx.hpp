#pragma once

#include "window.hpp"
#include "renderer.hpp"

struct bullet_loader final {
    gecs::entity operator()(gecs::commands cmds) const {
        auto entity = cmds.create();
    }
};

struct game_context final {
    std::unique_ptr<Window> window;
    std::unique_ptr<Renderer> renderer;
    bool shouldClose;
    SDL_Event event;

    static game_context create(const std::string& title, int w, int h);
};