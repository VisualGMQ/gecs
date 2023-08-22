#pragma once

#include "texture.hpp"

struct sprite final {
    Image image;
    Vector2 position;
    int depth;

    sprite(const Image& image, const Vector2& pos, int depth): image(image), position(pos), depth(depth) {}
};