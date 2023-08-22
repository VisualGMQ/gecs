#include "game_ctx.hpp"

GameContext GameContext::Create(const std::string& title, int w, int h) {
    auto window = std::make_unique<Window>(title, w, h);
    auto renderer = std::make_unique<Renderer>(*window);

    return GameContext{std::move(window), std::move(renderer), false};
}