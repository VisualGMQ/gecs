#include "renderer.hpp"

Renderer::Renderer(const Window& window): renderer_(SDL_CreateRenderer(window.window_.get(), -1, 0), RendererDestroy) {
}

void Renderer::SetColor(const SDL_Color& c) {
    SDL_SetRenderDrawColor(renderer_.get(), c.r, c.g, c.b, c.a);
}

void Renderer::Clear() {
    SDL_RenderClear(renderer_.get());
}

void Renderer::Present() {
    SDL_RenderPresent(renderer_.get());
}

void Renderer::DrawRect(const Rect& rect) {
    SDL_Rect r = {static_cast<int>(rect.x),
                  static_cast<int>(rect.y),
                  static_cast<int>(rect.w),
                  static_cast<int>(rect.h)};
    SDL_RenderDrawRect(renderer_.get(), &r);
}

void Renderer::FillRect(const Rect& rect) {
    SDL_Rect r = {static_cast<int>(rect.x),
                  static_cast<int>(rect.y),
                  static_cast<int>(rect.w),
                  static_cast<int>(rect.h)};
    SDL_RenderFillRect(renderer_.get(), &r);
}

void Renderer::DrawLine(const Vector2& p1, const Vector2& p2) {
    SDL_RenderDrawLineF(renderer_.get(), p1.x, p1.y, p2.x, p2.y);
}

void Renderer::DrawTexture(Texture& texture, const Rect& rect, int x, int y) {
    SDL_Rect src = {static_cast<int>(rect.x),
                    static_cast<int>(rect.y),
                    static_cast<int>(rect.w),
                    static_cast<int>(rect.h)};
    SDL_Rect dst = {x, y, static_cast<int>(rect.w), static_cast<int>(rect.h)};
    SDL_RenderCopy(renderer_.get(), texture.texture_.get(), &src, &dst);
}

void Renderer::DrawImage(const Image& image, const Vector2& position,
                         const std::optional<Vector2>& size) {
    if (!image.texture_) {
        return ;
    }
    SDL_FRect dst = {position.x, position.y,
                     size ? size->w : image.rect_.w,
                     size ? size->h : image.rect_.h};
    SDL_Rect src = {static_cast<int>(image.rect_.x),
                    static_cast<int>(image.rect_.y),
                    static_cast<int>(image.rect_.w),
                    static_cast<int>(image.rect_.h)};
    SDL_RenderCopyF(renderer_.get(), image.texture_->texture_.get(), &src, &dst);
}

void Renderer::DrawImage(const Image& image, const Vector2& position,
                         const Vector2& scale, float rotation) {
    SDL_FRect dst = {position.x, position.y,
                     std::abs(scale.x) * image.rect_.w,
                     std::abs(scale.y) * image.rect_.h};
    SDL_Rect src = {static_cast<int>(image.rect_.x),
                    static_cast<int>(image.rect_.y),
                    static_cast<int>(image.rect_.w),
                    static_cast<int>(image.rect_.h)};
    uint32_t flip = SDL_RendererFlip::SDL_FLIP_NONE;
    flip |= scale.x < 0 ? SDL_RendererFlip::SDL_FLIP_HORIZONTAL : 0;
    flip |= scale.y < 0 ? SDL_RendererFlip::SDL_FLIP_VERTICAL: 0;
    SDL_RenderCopyExF(renderer_.get(), image.texture_->texture_.get(), &src,
                      &dst, rotation, nullptr, static_cast<SDL_RendererFlip>(flip));
}
    
void Renderer::SetScale(const Vector2& scale) {
    SDL_RenderSetScale(renderer_.get(), scale.x, scale.y);
}