#pragma once
#include "pch.hpp"
#include "window.hpp"
#include "texture.hpp"

class Renderer final {
public:
    friend class Texture;

    Renderer(const Window&);

    void SetColor(const SDL_Color&);
    void Clear();
    void Present();
    void DrawRect(const SDL_Rect& rect);
    void FillRect(const SDL_Rect& rect);
    void DrawLine(const SDL_Point& p1, const SDL_Point& p2);
    void DrawTexture(Texture& texture, const SDL_Rect&, int x, int y);
    void DrawImage(const Image& image, const Vector2& position, const std::optional<Vector2>& size);
    void DrawImage(const Image& image, const Vector2& position, const Vector2& scale, float rotation = 0);
    void SetScale(const Vector2& scale);

private:
    inline static auto RendererDestroy = [](SDL_Renderer* renderer) {
        SDL_DestroyRenderer(renderer);
    };

    std::unique_ptr<SDL_Renderer, decltype(RendererDestroy)> renderer_;
};