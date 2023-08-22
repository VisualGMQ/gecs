#pragma once

#include "pch.hpp"

constexpr SDL_Color KeyColor = {50, 50, 50, 255};
constexpr float TankSpeed = 5;
constexpr int TankInitLife = 3;

static const Vector2 WindowSize = {1024.0f, 720.0f};
static const float ScaleFactor = 2.0;
static const Vector2 CanvaSize = WindowSize / 2.0;
static const float TankY = CanvaSize.h - 80;