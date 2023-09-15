#pragma once

#include "sigh.hpp"
#include "sink.hpp"

namespace gecs {

template <typename T, typename... Args>
class dispatcher {
public:
    using sigh_type = sigh<void(const T&, Args...)>;
    using cache_container = std::vector<T>;
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
        cache_.emplace_back(std::forward<Ts>(args)...);
    }

    void trigger_cached(Args... args) noexcept {
        for (auto& cache : cache_) {
            trigger(cache, std::forward<Args>(args)...);
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