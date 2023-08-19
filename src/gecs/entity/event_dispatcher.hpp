#pragma once

#include "gecs/signal/dispatcher.hpp"
#include "gecs/core/singlton.hpp"

namespace gecs {

template <typename T>
class event_dispatcher_singlton;

namespace internal {

template <typename T>
struct dispatcher_loader final {
    using type = event_dispatcher_singlton<T>;

    type operator()() const noexcept {
        return type{};
    }
};

template <typename T>
class event_dispatcher_singlton: public dispatcher<T>, public singlton<event_dispatcher_singlton<T>, false, internal::dispatcher_loader<T>> { };

}

template <typename T, typename WorldT>
class basic_event_dispatcher final {
public:
    using size_type = typename dispatcher<T>::size_type;
    using event_type = typename dispatcher<T>::event_type;

    basic_event_dispatcher(WorldT& world): world_(&world) { }

    auto sink() noexcept {
        return internal::event_dispatcher_singlton<T>::instance().sink();
    }

    //! @brief trigger all delegates immediately
    void trigger(const T& event) noexcept {
        return internal::event_dispatcher_singlton<T>::instance().trigger();
    }

    //! @brief trigger all delegates by cached events
    void update() noexcept {
        return internal::event_dispatcher_singlton<T>::instance().update();
    }

    //! @brief cache event
    template <typename... Args>
    void enqueue(Args&&... args) noexcept {
        internal::event_dispatcher_singlton<T>::instance().enqueue(std::forward<Args>(args)...);
    }

    void clear() noexcept {
        return internal::event_dispatcher_singlton<T>::instance().clear();
    }

    void clear_cache() noexcept {
        return internal::event_dispatcher_singlton<T>::instance().clear_cache();
    }

    size_type size() const noexcept {
        return internal::event_dispatcher_singlton<T>::instance().size();
    }

    bool empty() const noexcept {
        return internal::event_dispatcher_singlton<T>::instance().empty();
    }

private:
    WorldT* world_;
};

}
