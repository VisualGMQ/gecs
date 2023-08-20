#pragma once

#include "gecs/signal/dispatcher.hpp"
#include "gecs/core/singlton.hpp"

namespace gecs {

template <typename T, typename... Ts>
class event_dispatcher_singlton;

namespace internal {

template <typename T, typename... Ts>
struct dispatcher_loader final {
    using type = event_dispatcher_singlton<T, Ts...>;

    type operator()() const noexcept {
        return type{};
    }
};

template <typename T, typename... Ts>
struct event_dispatcher_singlton: public dispatcher<T, Ts...>, public singlton<event_dispatcher_singlton<T, Ts...>, false, internal::dispatcher_loader<T, Ts...>> {
    using dispatcher_type = dispatcher<T, Ts...>;
};

}

template <typename T, typename WorldT>
class basic_event_dispatcher final {
public:
    using dispatcher_singlton_type = internal::event_dispatcher_singlton<T, WorldT&>;
    using dispatcher_type = typename dispatcher_singlton_type::dispatcher_type;
    using size_type = typename dispatcher_type::size_type;
    using event_type = typename dispatcher_type::event_type;

    basic_event_dispatcher(WorldT& world): world_(&world) { }

    auto sink() noexcept {
        return dispatcher_singlton_type::instance().sink();
    }

    //! @brief trigger all delegates immediately
    void trigger(const T& event) noexcept {
        return dispatcher_singlton_type::instance().trigger();
    }

    //! @brief trigger all delegates by cached events
    void update() noexcept {
        return dispatcher_singlton_type::instance().update(*world_);
    }

    //! @brief cache event
    template <typename... Args>
    void enqueue(Args&&... args) noexcept {
        dispatcher_singlton_type::instance().enqueue(std::forward<Args>(args)...);
    }

    void clear() noexcept {
        return dispatcher_singlton_type::instance().clear();
    }

    void clear_cache() noexcept {
        return dispatcher_singlton_type::instance().clear_cache();
    }

    size_type size() const noexcept {
        return dispatcher_singlton_type::instance().size();
    }

    bool empty() const noexcept {
        return dispatcher_singlton_type::instance().empty();
    }

private:
    WorldT* world_;
};

}
