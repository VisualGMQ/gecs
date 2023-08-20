#include "game_ctx.hpp"

game_context game_context::create(const std::string& title, int w, int h) {
    auto window = std::make_unique<Window>(title, w, h);
    auto renderer = std::make_unique<Renderer>(*window);

    return game_context{std::move(window), std::move(renderer), false};
}