#include "texture.hpp"
#include "renderer.hpp"

Texture::Texture(Renderer* renderer, const std::string& filename,
                 const SDL_Color& keycolor,
                 std::optional<size_t> tilesheetIdx)
    : renderer_(renderer), texture_(nullptr, DestroyTexture), tilesheetIdx_(tilesheetIdx) {
  SDL_Surface* surface = SDL_LoadBMP(filename.c_str());
  if (!surface) {
    SDL_Log("[Texture]: load image %s failed: %s", filename.c_str(),
            SDL_GetError());
    texture_ = nullptr;
  } else {
    size_.Set(static_cast<float>(surface->w), static_cast<float>(surface->h));
    SDL_SetColorKey(
        surface, SDL_TRUE,
        SDL_MapRGB(surface->format, keycolor.r, keycolor.g, keycolor.b));
    texture_.reset(
        SDL_CreateTextureFromSurface(renderer->renderer_.get(), surface));
    if (!texture_) {
      SDL_Log("[Texture]: create texture from %s failed: %s", filename.c_str(),
              SDL_GetError());
    }
  }
}

Texture& TextureManager::Load(const std::string& name,
                              const std::string& filename,
                              const SDL_Color& keycolor) {
    return *textures_.emplace(name, std::make_unique<Texture>(renderer_, filename, keycolor, std::nullopt)).first->second;
}

Tilesheet& TextureManager::LoadTilesheet(const std::string& name,
                            const std::string& filename,
                            const SDL_Color& keycolor,
                            int col, int row) {
    auto& texture = textures_.emplace(name, std::make_unique<Texture>(renderer_, filename, keycolor, tilesheets_.size())).first->second;
    tilesheets_.emplace_back(std::make_unique<Tilesheet>(*texture, col, row));
    return *tilesheets_.back();
}

Tilesheet* TextureManager::FindTilesheet(const std::string& name) {
  if (auto it = textures_.find(name); it != textures_.end()) {
      auto& texture = it->second;
      if (!texture->IsTilesheet()) {
        SDL_Log("[TextureManager][WARN]: texture %s is a pure texture", name.c_str());
      }
      return tilesheets_[texture->GetTilesheetIdx().value()].get();
    }
    return nullptr;
}


Texture* TextureManager::Find(const std::string& name) {
    if (auto it = textures_.find(name); it != textures_.end()) {
      if (it->second->IsTilesheet()) {
        SDL_Log("[TextureManager][WARN]: texture %s is a tilesheet", name.c_str());
      }
      return it->second.get();
    }
    return nullptr;
}

Image::Image(Texture& texture): texture_(&texture), rect_({0, 0}, texture.GetSize()) { }

Image::Image(Texture& texture, Rect rect): texture_(&texture), rect_(rect) { }

Tilesheet::Tilesheet(Texture& texture, int col, int row)
    : texture_(texture),
      row_(row),
      col_(col),
      tileSize_(texture.GetSize().w / col, texture.GetSize().h / row) {}