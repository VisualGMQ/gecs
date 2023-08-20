#pragma once

#include "pch.hpp"

inline auto WindowDestroy = [](SDL_Window* window) {
    SDL_DestroyWindow(window);
};

class Window final {
public:
    friend class Renderer;

    Window(const std::string& title, int w, int h);

private:
    std::unique_ptr<SDL_Window, decltype(WindowDestroy)> window_;
};