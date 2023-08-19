#pragma once

#include "sigh.hpp"

namespace gecs {

template <typename T>
class dispatcher {
public:
    using sigh_type = sigh<void(const T&)>;
    using cache_container = std::vector<T>;
    using size_type = typename sigh_type::size_type;
    using event_type = T;

    auto sink() noexcept {
        return sink(sighs_);
    }

    //! @brief trigger all delegates immediately
    void trigger(const T& event) noexcept {
        sighs_.trigger(event);
    }

    //! @brief trigger all delegates by cached events
    void update() noexcept {
        for (auto& event : cache_) {
            sighs_.trigger(event);
        }
        clear_cache();
    }

    //! @brief cache event
    template <typename... Args>
    void enqueue(Args&&... args) noexcept {
        cache_.emplace_back(std::forward<Args>(args)...);
    }

    void clear() noexcept {
        sighs_.clear();
    }

    void clear_cache() noexcept {
        cache_.clear();
    }

    size_type size() const noexcept {
        return sighs_.size();
    }

    bool empty() const noexcept {
        return sighs_.empty();
    }

    virtual ~dispatcher() = default;

private:
    sigh_type sighs_;
    cache_container cache_;
};

}