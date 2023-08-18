#pragma once

#include "delegate.hpp"

namespace gecs {

template <typename T>
class sigh;

template <typename Ret, typename... Args>
class sigh<Ret(Args...)> {
public:
    using delegate_type = delegate<Ret(Args...)>;
    using container_type = std::vector<delegate_type>;
    using size_type = typename container_type::size_type;

    size_type size() const noexcept {
        return delegates_.size();
    }

    bool empty() const noexcept {
        return delegates_.empty();
    }

    void add(const delegate_type& d) noexcept {
        delegates_.push_back(d);
    }

    void add(delegate_type&& d) noexcept {
        delegates_.push_back(std::move(d));
    }

    void trigger(Args... args) noexcept {
        for (auto& delegate : delegates_) {
            delegate(std::forward<Args>(args)...);
        }
    }

    auto& operator+=(const delegate_type& d) noexcept {
        add(d);
        return *this;
    }

    auto& operator+=(delegate_type&& d) noexcept {
        add(std::move(d));
        return *this;
    }

    void clear() noexcept {
        delegats_.clear();
    }

private:
    container_type delegates_;
};

}