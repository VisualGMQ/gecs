#pragma once
#include "pch.hpp"

class Renderer;

class Texture final {
public:
    friend class Renderer;

    Texture(Renderer* renderer, const std::string& filename,
            const SDL_Color& keycolor, std::optional<size_t> tilesheetIdx);

    const auto& GetSize() const { return size_; }

    bool IsTilesheet() const { return tilesheetIdx_.has_value(); }

    auto GetTilesheetIdx() const { return tilesheetIdx_; }

private:
    inline static auto DestroyTexture = [](SDL_Texture* texture) {
        SDL_DestroyTexture(texture);
    };
    std::optional<size_t> tilesheetIdx_;

    Renderer* renderer_;
    Vector2 size_;
    std::unique_ptr<SDL_Texture, decltype(DestroyTexture)> texture_;
};

class Image final {
public:
    friend class Renderer;

    Image() : texture_(nullptr) {}

    Image(const Texture& texture);
    Image(const Texture& texture, Rect rect);

    auto Size() const { return rect_.size; }

private:
    const Texture* texture_;
    Rect rect_;
};

class Tilesheet final {
public:
    Tilesheet(Texture& texture, int col, int row);

    int GetRow() const { return row_; }

    int GetCol() const { return col_; }

    auto& GetTileSize() const { return tileSize_; }

    Image Get(int col, int row) const {
        return Image(texture_,
                     Rect({col * tileSize_.w, row * tileSize_.h}, tileSize_));
    }

private:
    Texture& texture_;
    int row_;
    int col_;
    Size tileSize_;
};

class TextureManager final {
public:
    Texture& Load(const std::string& name, const std::string& filename,
                  const SDL_Color& keycolor);
    Tilesheet& LoadTilesheet(const std::string& name,
                             const std::string& filename,
                             const SDL_Color& keycolor, int col, int row);
    const Texture* Find(const std::string& name) const;
    const Tilesheet* FindTilesheet(const std::string& name) const;

    TextureManager(Renderer* renderer) : renderer_(renderer) {}

    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

private:
    Renderer* renderer_;
    std::unordered_map<std::string, std::unique_ptr<Texture>> textures_;
    std::vector<std::unique_ptr<Tilesheet>> tilesheets_;
};