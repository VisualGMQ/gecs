#pragma once

#include "pch.hpp"

class Ticker final {
public:
    Ticker(int duration): duration_(duration), tick_(0) {}

    void Update() {
        if (tick_ < duration_) {
            tick_ ++;
        }
    }

    bool isFinish() const {
        return tick_ >= duration_;
    }

    void Reset() {
        tick_ = 0;
    }

private:
    int duration_;
    int tick_;
};