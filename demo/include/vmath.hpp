#pragma once

#include <cmath>
#include <limits>
#include <memory>

struct Vector2 final {
    union {
        float data[2];

        struct {
            float x, y;
        };

        struct {
            float w, h;
        };
    };

    Vector2() { Set(0, 0); }

    Vector2(float x, float y) { Set(x, y); }

    explicit Vector2(float value) { Set(value, value); }

    void Set(float x, float y) {
        this->x = x;
        this->y = y;
    }

    bool operator==(const Vector2& o) const {
        return this->x == o.x && this->y == o.y;
    }

    bool operator!=(const Vector2& o) const { return !(*this == o); }

    Vector2 operator+(const Vector2& o) const {
        return Vector2{o.x + x, o.y + y};
    }

    Vector2 operator-(const Vector2& o) const {
        return Vector2{x - o.x, y - o.y};
    }

    Vector2 operator*(const Vector2& o) const {
        return Vector2{o.x * x, o.y * y};
    }

    Vector2 operator/(const Vector2& o) const {
        return Vector2{x / o.x, y / o.y};
    }

    Vector2 operator*(float value) const {
        return Vector2{x * value, y * value};
    }

    Vector2 operator/(float value) const {
        return Vector2{x / value, y / value};
    }

    Vector2& operator+=(const Vector2& o) {
        *this = *this + o;
        return *this;
    }

    Vector2& operator-=(const Vector2& o) {
        *this = *this - o;
        return *this;
    }

    Vector2& operator*=(const Vector2& o) {
        *this = *this * o;
        return *this;
    }

    Vector2& operator/=(const Vector2& o) {
        *this = *this / o;
        return *this;
    }

    float LengthSqrd() const { return Dot(*this); }

    float Length() const { return std::sqrt(LengthSqrd()); }

    float Dot(const Vector2& o) const { return x * o.x + y * o.y; }

    Vector2 Normalize() const {
        float len = Length();
        if (std::abs(len) <= std::numeric_limits<float>::epsilon()) {
            return *this;
        }
        return *this / Length();
    }
};

inline Vector2 operator*(float value, const Vector2& v) {
    return v * value;
}

inline Vector2 operator-(const Vector2& v) {
    return {-v.x, -v.y};
}

using Size = Vector2;

template <typename T>
class Matrix final {
public:
    Matrix(int w, int h)
        : data_(std::unique_ptr<T[]>(new T[w * h])), w_(w), h_(h) {}

    void Fill(const T& value) {
        for (int i = 0; i < w_ * h_; i++) {
            GetByIndex(i) = value;
        }
    }

    T& Get(int x, int y) { return data_.get()[x + y * w_]; }

    const T& Get(int x, int y) const { return data_.get()[x + y * w_]; }

    T& GetByIndex(int idx) { return data_.get()[idx]; }

    int MaxSize() const { return w_ * h_; }

    int Width() const { return w_; }

    int Height() const { return h_; }

    bool IsIn(int x, int y) const {
        return x >= 0 && x < w_ && y >= 0 && y < h_;
    }

private:
    std::unique_ptr<T[]> data_;
    int w_;
    int h_;
};

struct Rect final {
    union {
        SDL_FRect sdlrect;

        struct {
            Vector2 position;
            Size size;
        };

        struct {
            float x, y, w, h;
        };
    };

    Rect() : x(0), y(0), w(0), h(0) {}

    Rect(const Vector2& pos, const Vector2& size) {
        this->position = pos;
        this->size = size;
    }

    Rect(float x, float y, float w, float h) {
        this->position.Set(x, y);
        this->size.Set(w, h);
    }

    bool IsIntersect(const Rect& rect) const {
        return !(position.x + size.x <= rect.position.x ||
                 position.x >= rect.position.x + rect.size.x ||
                 position.y + size.y <= rect.position.y ||
                 position.y >= rect.position.y + rect.size.y);
    }

    Vector2 Center() const { return position + size / 2.0; }
};