#pragma once

#include "vmath.hpp"

struct RigidBody final {
    Vector2 velocity;
    Rect collide;
};

inline bool IsRectIntersect(const Rect& r1, const Rect& r2) {
    return !(
        r1.x >= r2.x + r2.w ||
        r1.y >= r2.y + r2.h ||
        r1.x + r1.w <= r2.x ||
        r1.y + r1.h <= r2.y
    );
}

//! @brief get collide time by velocity, rect must colliding
inline float RectCollidTime(const Rect& src, const Rect& dst, const Vector2& src_vel) {
    Vector2 old_pos = src.position - src_vel;
    Vector2 half_src_size = src.size / 2.0;
    float x_percent = src_vel.x == 0 ? 0 : std::min((dst.position.x + dst.size.w + half_src_size.w) / src_vel.x,
                               (dst.position.x - half_src_size.w) / src_vel.x);
    float y_percent = src_vel.y == 0 ? 0 : std::min((dst.position.y + dst.size.h + half_src_size.h) / src_vel.y,
                               (dst.position.y - half_src_size.h) / src_vel.y);
    return std::max(x_percent, y_percent);
}
