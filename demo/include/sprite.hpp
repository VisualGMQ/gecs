#pragma once

#include "texture.hpp"

struct Sprite final {
    Image image;
    Vector2 position;
    int depth;

    Sprite(const Image& image, const Vector2& pos, int depth)
        : image(image), position(pos), depth(depth) {}
};