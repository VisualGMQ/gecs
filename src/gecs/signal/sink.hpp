#pragma once

#include "sigh.hpp"

#include <algorithm>

namespace gecs {

template <typename SighT>
class sink final {
public:
    using sigh_type = SighT;
    using delegate_type = typename sigh_type::delegate_type;
    using delegate_pointer_type = typename delegate_type::delegate_pointer_type;

    sink(sigh_type& sigh) noexcept: sigh_(&sigh) {}

    template <auto Func>
    void add() noexcept {
        delegate_type d;
        d.connect<Func>();
        sigh_->delegates_.emplace_back(std::move(d));
    }

    void add(delegate_type d) noexcept {
        sigh_->delegates_.emplace_back(d);
    }

    void add(delegate_pointer_type d) noexcept {
        delegate_type d;
        d.connect(d);
        sigh_->delegates_.emplace_back(std::move(d));
    }

    template <typename Payload>
    void add(delegate_pointer_type d, Payload& payload) noexcept {
        delegate_type d;
        d.connect(d, payload);
        sigh_->delegates_.emplace_back(std::move(d));
    }

    template <typename Payload>
    void add(delegate_pointer_type d, Payload* payload) noexcept {
        delegate_type d;
        d.connect(d, payload);
        sigh_->delegates_.emplace_back(std::move(d));
    }

    template <auto Func, typename Payload>
    void add(Payload& payload) noexcept {
        delegate_type d;
        d.connect(payload);
        sigh_->delegates_.emplace_back(std::move(d));
    }

    template <auto Func, typename Payload>
    void add(Payload* payload) noexcept {
        delegate_type d;
        d.connect(payload);
        sigh_->delegates_.emplace_back(std::move(d));
    }

    template <auto Func, size_t... Index>
    void add(std::index_sequence<Index...> indices) noexcept {
        delegate_type d;
        d.connect<Func>(indices);
        sigh_->delegates_.emplace_back(std::move(d));
    }

    template <auto Func, typename Payload, size_t... Index>
    void add(Payload& payload, std::index_sequence<Index...> indices) noexcept {
        delegate_type d;
        d.connect<Func>(payload, indices);
        sigh_->delegates_.emplace_back(std::move(d));
    }

    template <auto Func, typename Payload, size_t... Index>
    void add(Payload* payload, std::index_sequence<Index...> indices) noexcept {
        delegate_type d;
        d.connect<Func>(payload, indices);
        sigh_->delegates_.emplace_back(std::move(d));
    }

    void remove(const delegate_type& d) noexcept {
        auto& delegates = sigh_->delegates_;
        auto it = std::remove_if(delegates.begin(), delegates.end(), [&d](auto& dlg) {
            return dlg->fn() == d;
        });
        delegates.erase(it, delegates.end());
    }

    template <typename Payload>
    void remove(const Payload& payload) noexcept {
        auto& delegates = sigh_->delegates_;
        auto it = std::remove_if(delegates.begin(), delegates.end(), [&d](auto& dlg) {
            return dlg->payload() == &payload;
        });
        delegates.erase(it, delegates.end());
    }

    void clear() noexcept {
        sigh_->delegates_.clear();
    }

private:
    sigh_type* sigh_; 
};

}