#pragma once

#include "texture.hpp"

class Frame final {
public:
    Frame(Image image, int duration): image_(image), duration_(duration) {}

    const Image& GetImage() const { return image_; }
    int Duration() const { return duration_; }

private:
    Image image_;
    int duration_;
};

class Animation final {
public:
    Animation() = default;
    Animation(const std::vector<Frame>& frames): frames_(frames) { }

    template <typename... Args>
    auto& Add(Args&&... args) {
        frames_.emplace_back(std::forward<Args>(args)...);
        return *this;
    }

    auto& SetLoop(int loop) {
        loop_ = loop;
        return *this;
    }

    int GetLoop() const {
        return loop_;
    }

    const Image& CurImage() const {
        return frames_[cur_frame_].GetImage();
    }

    void Update(){
        if (!IsPlaying()) {
            return ; 
        }

        if (IsFinish()) {
            if (loop_ != 0) {
                if (loop_ > 0) {
                    loop_ --;
                }
                cur_frame_ = 0;
                tick_ = 0;
            } else {
                playing_ = false;
            }
        }

        if (!IsFinish()) {
            int duration = frames_[cur_frame_].Duration();
            if (tick_ >= duration) {
                tick_ -= duration;
                cur_frame_ ++;
            }
            tick_ ++;
        }
    }

    bool IsPlaying() const {
        return playing_;
    }

    auto& Play() {
        playing_ = true;
        return *this;
    }

    auto& Stop() {
        playing_ = false;
        return *this;
    }

    bool IsFinish() const {
        return cur_frame_ == int(frames_.size()) - 1 &&
                tick_ == frames_[cur_frame_].Duration();
    }

    int CurFrameIdx() const {
        return cur_frame_;
    }

private:
    std::vector<Frame> frames_;
    int cur_frame_ = 0;
    int tick_ = 0;
    int loop_ = 0;
    bool playing_ = false;
};

class AnimManager final {
public:
    auto& Create(const std::string name, const Animation& anim) {
        return anims_.emplace(name, anim).first->second;
    }

    auto& Create(const std::string name, const std::vector<Frame>& frames) {
        return anims_.emplace(name, frames).first->second;
    }

    const Animation* Find(const std::string name) const {
        if (auto it = anims_.find(name); it != anims_.end()) {
            return &it->second;
        } else {
            return nullptr;
        }
    }

    Animation* Find(const std::string name) {
        return const_cast<Animation*>(std::as_const(*this).Find(name));
    }

private:
    std::unordered_map<std::string, Animation> anims_;
};