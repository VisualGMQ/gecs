#pragma once

#include "pch.hpp"

class ticker final {
public:
    ticker(int duration): duration_(duration), tick_(0) {}

    void update() {
        if (tick_ < duration_) {
            tick_ ++;
        }
    }

    bool is_end() const {
        return tick_ >= duration_;
    }

    void reset() {
        tick_ = 0;
    }

private:
    int duration_;
    int tick_;
};