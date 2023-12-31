#pragma once
#include "pch.hpp"
#include "texture.hpp"
#include "window.hpp"

class Renderer final {
public:
    friend class Texture;

    Renderer(const Window&);

    void SetColor(const SDL_Color&);
    void Clear();
    void Present();
    void DrawRect(const Rect& rect);
    void FillRect(const Rect& rect);
    void DrawLine(const Vector2& p1, const Vector2& p2);
    void DrawTexture(Texture& texture, const Rect&, int x, int y);
    void DrawImage(const Image& image, const Vector2& position,
                   const std::optional<Vector2>& size);
    void DrawImage(const Image& image, const Vector2& position,
                   const Vector2& scale, float rotation = 0);
    void SetScale(const Vector2& scale);

private:
    inline static auto RendererDestroy = [](SDL_Renderer* renderer) {
        SDL_DestroyRenderer(renderer);
    };

    std::unique_ptr<SDL_Renderer, decltype(RendererDestroy)> renderer_;
};