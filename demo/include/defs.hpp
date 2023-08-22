#pragma once

#include "game_ctx.hpp"
#include "anim.hpp"
#include "sprite.hpp"
#include "physics.hpp"
#include "consts.hpp"

enum DepthLayer {
    LandDepth = 0,
    TankDepth,
    FallingStoneDepth,
    BulletDepth,
    BombDepth,
};

// a tag for bomb animation
struct Bomb { };

// a tag for land
struct Land {};

// a tag for falling stone entity
struct FallingStone {
    int hp;
};
// a tag for Bullet entity
struct Bullet {
    int damage;
};
// a tag for player's tank entity
struct Tank {
    int hp;
    int score;
};

inline gecs::entity CreateBullet(const Vector2& init_pos, const Vector2& vel, gecs::commands cmds, gecs::resource<AnimManager> anim_mgr) {
    auto entity = cmds.create();
    cmds.emplace<Animation>(entity, *(anim_mgr->Find("shell_fly")));
    cmds.emplace<Sprite>(entity, Image{}, init_pos, DepthLayer::BulletDepth);
    cmds.emplace<Bullet>(entity, Bullet{1});
    cmds.emplace<RigidBody>(entity, RigidBody{vel, Rect{10, 0, 12, 20}});
    return entity;
}

inline gecs::entity CreateFallingStone(const Vector2& init_pos, const Vector2& vel, gecs::commands cmds, gecs::resource<AnimManager> anim_mgr) {
    auto entity = cmds.create();
    cmds.emplace<Animation>(entity, *(anim_mgr->Find("falling_stone")));
    cmds.emplace<Sprite>(entity, Image{}, init_pos, DepthLayer::FallingStoneDepth);
    cmds.emplace<FallingStone>(entity, FallingStone{1});
    cmds.emplace<RigidBody>(entity, RigidBody{vel, Rect{4, 35, 24, 26}});
    return entity;
}

inline RigidBody& CreateBombAnim(const Vector2& init_pos, const Vector2& vel, gecs::commands cmds, gecs::resource<AnimManager> anim_mgr) {
    auto entity = cmds.create();
    cmds.emplace<Animation>(entity, *(anim_mgr->Find("bomb")));
    cmds.emplace<Sprite>(entity, Image{}, init_pos, DepthLayer::BombDepth);
    cmds.emplace<Bomb>(entity);
    return cmds.emplace<RigidBody>(entity, RigidBody{vel, Rect{}});
}
