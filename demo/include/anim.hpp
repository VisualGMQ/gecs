#pragma once

#include "texture.hpp"

class frame final {
public:
    frame(Image image, int duration): image_(image), duration_(duration) {}

    const Image& image() const { return image_; }
    int duration() const { return duration_; }

private:
    Image image_;
    int duration_;
};

class animation final {
public:
    template <typename... Args>
    auto& add(Args&&... args) {
        frames_.emplace_back(std::forward<Args>(args)...);
        return *this;
    }

    auto& set_loop(int loop) {
        loop_ = loop;
        return *this;
    }

    int get_loop() const {
        return loop_;
    }

    const Image& cur_image() const {
        return frames_[cur_frame_].image();
    }

    void update(){
        if (!is_playing()) {
            return ; 
        }

        if (is_end()) {
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

        if (!is_end()) {
            int duration = frames_[cur_frame_].duration();
            if (tick_ >= duration) {
                tick_ -= duration;
                cur_frame_ ++;
            }
            tick_ ++;
        }
    }

    bool is_playing() const {
        return playing_;
    }

    auto& play() {
        playing_ = true;
        return *this;
    }

    auto& stop() {
        playing_ = false;
        return *this;
    }

    bool is_end() const {
        return cur_frame_ == int(frames_.size()) - 1 &&
                tick_ == frames_[cur_frame_].duration();
    }

    int cur_frame_idx() const {
        return cur_frame_;
    }

private:
    std::vector<frame> frames_;
    int cur_frame_ = 0;
    int tick_ = 0;
    int loop_ = 0;
    bool playing_ = false;
};