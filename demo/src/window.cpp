#include "window.hpp"

Window::Window(const std::string &title, int w, int h) : window_(SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED,
                                                                                  SDL_WINDOWPOS_CENTERED, w, h, SDL_WINDOW_SHOWN), WindowDestroy)
{
}