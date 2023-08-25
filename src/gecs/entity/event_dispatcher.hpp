#pragma once

#include "gecs/core/singlton.hpp"
#include "gecs/core/utility.hpp"
#include "gecs/entity/system_constructor.hpp"
#include "gecs/signal/dispatcher.hpp"

#include <cstdint>

namespace gecs {

template <typename, typename>
class basic_event_dispatcher;

template <typename, typename>
class basic_event_dispatcher_wrapper;

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

template <typename T, typename RegistryT>
struct event_dispatcher_sink final {
    using basic_event_dispatcher_type =
        ::gecs::basic_event_dispatcher<T, RegistryT>;
    using sigh_type =
        typename basic_event_dispatcher_type::dispatcher_type::sigh_type;
    using sink_type = sink<sigh_type>;

    event_dispatcher_sink(sink_type sink) : sink_(sink) {}

    template <auto Func>
    void add() noexcept {
        typename sink_type::delegate_type d;

        constexpr void (*f)(const T&, RegistryT&) =
            internal::system_constructor_v<Func, RegistryT, const T&>;
        d.template connect<f>();
        sink_.add(std::move(d));
    }

    template <auto Func>
    void remove() noexcept {
        constexpr void (*f)(const T&, RegistryT&) =
            internal::system_constructor_v<Func, RegistryT, const T&>;
        sink_.template remove<f>();
    }

    void clear() noexcept { sink_.clear(); }

private:
    sink_type sink_;
};

}  // namespace internal

struct event_dispatcher_base { 
    virtual void trigger_cached() = 0;
    virtual void clear_cache() = 0;
    virtual ~event_dispatcher_base() = default;
};

template <typename T, typename RegistryT>
struct event_dispatcher_type_for {
    using type = basic_event_dispatcher<T, RegistryT>;
};

template <typename T, typename RegistryT>
using event_dispatcher_type_for_t = typename event_dispatcher_type_for<T, RegistryT>::type;

template <typename T, typename RegistryT>
class basic_event_dispatcher final : public dispatcher<T, RegistryT&>, public event_dispatcher_base {
public:
    using dispatcher_type = dispatcher<T, RegistryT&>;
    using size_type = typename dispatcher_type::size_type;
    using event_type = typename dispatcher_type::event_type;
    using sink_type = internal::event_dispatcher_sink<T, RegistryT>;

    basic_event_dispatcher(RegistryT& reg) : reg_(&reg) {}

    auto sink() noexcept {
        return sink_type(dispatcher_type::sink());
    }

    //! @brief trigger all delegates immediately
    void trigger(const T& event) noexcept {
        return dispatcher_type::trigger(event, *reg_);
    }

    //! @brief trigger all delegates by cached events
    void update() noexcept {
        return dispatcher_type::update(*reg_);
    }

    void trigger_cached() noexcept override {
        dispatcher_type::trigger_cached(*reg_);
    }

    //! @brief cache event
    template <typename... Args>
    void enqueue(Args&&... args) noexcept {
        dispatcher_type::enqueue(
            std::forward<Args>(args)...);
    }

    void clear() noexcept {
        dispatcher_type::clear();
    }

    void clear_cache() noexcept override {
        dispatcher_type::clear_cache();
    }

    size_type size() const noexcept {
        return dispatcher_type::size();
    }

    bool empty() const noexcept {
        return dispatcher_type::empty();
    }

private:
    RegistryT* reg_;
};

template <typename T, typename RegistryT>
class basic_event_dispatcher_wrapper {
public:
    using event_dispatcher_type = basic_event_dispatcher<T, RegistryT>;

    using dispatcher_type = typename event_dispatcher_type::dispatcher_type;
    using size_type = typename event_dispatcher_type::size_type;
    using event_type = typename event_dispatcher_type::event_type;
    using sink_type = typename event_dispatcher_type::sink_type;

    basic_event_dispatcher_wrapper(event_dispatcher_type& event_dispatcher) : event_dispatcher_(event_dispatcher) {}

    auto sink() noexcept {
        return event_dispatcher_.sink();
    }

    //! @brief trigger all delegates immediately
    void trigger(const T& event) noexcept {
        return event_dispatcher_.trigger(event);
    }

    //! @brief trigger all delegates by cached events
    void update() noexcept {
        return event_dispatcher_.update();
    }

    void trigger_cached() noexcept {
        event_dispatcher_.trigger_cached();
    }

    //! @brief cache event
    template <typename... Args>
    void enqueue(Args&&... args) noexcept {
        event_dispatcher_.enqueue(std::forward<Args>(args)...);
    }

    void clear() noexcept {
        event_dispatcher_.clear();
    }

    void clear_cache() noexcept {
        event_dispatcher_.clear_cache();
    }

    size_type size() const noexcept {
        return event_dispatcher_.size();
    }

    bool empty() const noexcept {
        return event_dispatcher_.empty();
    }

private:
    event_dispatcher_type& event_dispatcher_;
};

}  // namespace gecs
