#pragma once

#include "gecs/core/singlton.hpp"
#include "gecs/core/utility.hpp"
#include "gecs/entity/system_constructor.hpp"
#include "gecs/signal/dispatcher.hpp"

namespace gecs {

template <typename T, typename WorldT>
class basic_event_dispatcher;

namespace internal {

template <typename T, typename... Ts>
struct event_dispatcher_singlton;

template <typename T, typename... Ts>
struct dispatcher_loader final {
    using type = event_dispatcher_singlton<T, Ts...>;

    type operator()() const noexcept { return type{}; }
};

template <typename T, typename... Ts>
struct event_dispatcher_singlton
    : public dispatcher<T, Ts...>,
      public singlton<event_dispatcher_singlton<T, Ts...>, false,
                      internal::dispatcher_loader<T, Ts...>> {
    using dispatcher_type = dispatcher<T, Ts...>;
};

template <typename T, typename WorldT>
struct event_dispatcher_sink final {
    using basic_event_dispatcher_type =
        ::gecs::basic_event_dispatcher<T, WorldT>;
    using sigh_type =
        typename basic_event_dispatcher_type::dispatcher_type::sigh_type;
    using sink_type = sink<sigh_type>;

    event_dispatcher_sink(sink_type sink) : sink_(sink) {}

    template <auto Func>
    void add() noexcept {
        typename sink_type::delegate_type d;

        constexpr void (*f)(const T&, WorldT&) =
            internal::system_constructor_v<Func, WorldT, const T&>;
        // show_tmpl_error<decltype(f)> {};
        d.template connect<f>();
        sink_.add(std::move(d));
    }

    template <auto Func>
    void remove() noexcept {
        sink_.template remove<Func>();
    }

    void clear() noexcept { sink_.clear(); }

private:
    sink_type sink_;
};

}  // namespace internal

template <typename T, typename WorldT>
class basic_event_dispatcher final {
public:
    using dispatcher_singlton_type =
        internal::event_dispatcher_singlton<T, WorldT&>;
    using dispatcher_type = typename dispatcher_singlton_type::dispatcher_type;
    using size_type = typename dispatcher_type::size_type;
    using event_type = typename dispatcher_type::event_type;
    using sink_type = internal::event_dispatcher_sink<T, WorldT>;

    basic_event_dispatcher(WorldT& world) : world_(&world) {}

    auto sink() noexcept {
        return sink_type(dispatcher_singlton_type::instance().sink());
    }

    //! @brief trigger all delegates immediately
    void trigger(const T& event) noexcept {
        return dispatcher_singlton_type::instance().trigger(event, *world_);
    }

    //! @brief trigger all delegates by cached events
    void update() noexcept {
        return dispatcher_singlton_type::instance().update(*world_);
    }

    //! @brief cache event
    template <typename... Args>
    void enqueue(Args&&... args) noexcept {
        dispatcher_singlton_type::instance().enqueue(
            std::forward<Args>(args)...);
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

}  // namespace gecs
