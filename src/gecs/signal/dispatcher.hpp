#pragma once

#include "sigh.hpp"
#include "sink.hpp"
#include <memory>

namespace gecs {

template <typename T, typename... Args>
class dispatcher {
public:
    using sigh_type = sigh<void(const T&, Args...)>;
    using cache_container = std::vector<std::unique_ptr<T>>;
    using size_type = typename sigh_type::size_type;
    using event_type = T;

    virtual ~dispatcher() = default;

    auto sink() noexcept { return ::gecs::sink{sigh_}; }

    //! @brief trigger all delegates immediately
    void trigger(const T& event, Args... args) noexcept {
        sigh_.trigger(event, std::forward<Args>(args)...);
    }

    //! @brief trigger all delegates by cached events
    void update(Args... args) noexcept {
        trigger_cached(std::forward<Args>(args)...);
        clear_cache();
    }

    //! @brief cache event
    template <typename... Ts>
    void enqueue(Ts&&... args) noexcept {
        cache_.emplace_back(std::make_unique<T>(std::forward<Ts>(args)...));
    }

    void trigger_cached(Args... args) noexcept {
        while (!cache_.empty()) {
            trigger(*cache_[0], std::forward<Args>(args)...);
            std::swap(cache_[0], cache_.back());
            cache_.pop_back();
        }
    }

    void clear() noexcept { sigh_.clear(); }

    void clear_cache() noexcept { cache_.clear(); }

    size_type size() const noexcept { return sigh_.size(); }

    bool empty() const noexcept { return sigh_.empty(); }

private:
    sigh_type sigh_;
    cache_container cache_;
};

}  // namespace gecs