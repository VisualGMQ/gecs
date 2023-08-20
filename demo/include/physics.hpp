#pragma once

#include "vmath.hpp"

struct rigidbody final {
    Vector2 velocity;
    Vector2 position;
    Rect collide;
};