#pragma once

#include "delegate.hpp"

namespace gecs {

template <typename SighT>
class sink;

template <typename T>
class sigh;

/**
 * @brief signal handler
 * 
 * a signal handler to simulate C# delegate. But trigger all delegates in insert order
 * 
 * @tparam Ret 
 * @tparam Args 
 */
template <typename Ret, typename... Args>
class sigh<Ret(Args...)> final {
public:
    using delegate_type = delegate<Ret(Args...)>;
    using container_type = std::vector<delegate_type>;
    using size_type = typename container_type::size_type;
    using self_type = sigh<Ret(Args...)>;

    friend class sink<self_type>;

    size_type size() const noexcept {
        return delegates_.size();
    }

    bool empty() const noexcept {
        return delegates_.empty();
    }

    //! @brief call all delegates with arguments
    void trigger(Args... args) noexcept {
        for (auto& delegate : delegates_) {
            delegate(std::forward<Args>(args)...);
        }
    }

    auto& operator+=(const delegate_type& d) noexcept {
        delegates_.emplace_back(d);
        return *this;
    }

    auto& operator+=(delegate_type&& d) noexcept {
        delegates_.emplace_back(std::move(d));
        return *this;
    }

    void clear() noexcept {
        delegates_.clear();
    }

private:
    container_type delegates_;
};

}