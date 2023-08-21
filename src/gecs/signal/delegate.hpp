#pragma once

#include "gecs/core/utility.hpp"
#include "gecs/core/type_list.hpp"

#include <utility>
#include <functional>

namespace gecs {

template <typename Func>
class delegate;

/**
 * @brief delegate
 * 
 * a delegate class for Observer Pattern
 * 
 * @tparam Ret 
 * @tparam Args 
 */
template <typename Ret, typename... Args>
class delegate<Ret(Args...)> final {
public:
    using delegate_fn_type = Ret(const void*, Args...);
    using delegate_pointer_type = Ret(*)(const void*, Args...);
    using fn_type = Ret(Args...);
    using fn_pointer_type = Ret(Args...);

    delegate() = default;

    template <auto Func>
    void connect() noexcept {
        fn_ = wrap<Func>(std::make_index_sequence<sizeof...(Args)>{});
    }

    void connect(delegate_pointer_type d) {
        payload_ = nullptr;
        fn_ = d;
    }

    template <typename Payload>
    void connect(delegate_pointer_type d, Payload& payload) {
        payload_ = &payload;
        fn_ = d;
    }

    template <typename Payload>
    void connect(delegate_pointer_type d, Payload* payload) {
        payload_ = payload;
        fn_ = d;
    }

    template <auto Func, typename Payload>
    void connect(Payload& payload) {
        fn_ = wrap<Func>(payload, std::make_index_sequence<sizeof...(Args)>{});
    }

    template <auto Func, typename Payload>
    void connect(Payload* payload) {
        fn_ = wrap<Func>(payload, std::make_index_sequence<sizeof...(Args)>{});
    }

    template <auto Func, size_t... Index>
    void connect(std::index_sequence<Index...> indices) noexcept {
        fn_ = wrap<Func>(indices);
    }

    template <auto Func, typename Payload, size_t... Index>
    void connect(Payload& payload, std::index_sequence<Index...> indices) {
        fn_ = wrap<Func>(payload, indices);
    }

    template <auto Func, typename Payload, size_t... Index>
    void connect(Payload* payload, std::index_sequence<Index...> indices) {
        fn_ = wrap<Func>(payload, indices);
    }

    Ret operator()(Args... args) {
        return std::invoke(fn_, payload_, std::forward<Args>(args)...);
    }

    void reset() {
        payload_ = nullptr;
        fn_ = nullptr;
    }

    operator bool() const {
        return fn_ != nullptr;
    }

    void release() {
        payload_ = nullptr;
        fn_ = nullptr;
    }

    bool operator==(const delegate& o) const {
        return o.payload_ == payload_ && o.fn_ == fn_;
    }

    bool operator!=(const delegate& o) const {
        return !(*this == o);
    }

    const void* payload() const noexcept {
        return payload_;
    }

    auto fn() const noexcept {
        return fn_;
    }

    ~delegate() {
        release();
    }

private:
    const void* payload_ = nullptr;
    delegate_pointer_type fn_ = nullptr; 

    template <auto Func, size_t... Index>
    delegate_pointer_type wrap(std::index_sequence<Index...>) {
        payload_ = nullptr;
        using args_list = type_list<Args...>;
        if constexpr (std::is_invocable_r_v<Ret, decltype(Func), type_list_element_t<Index, args_list>...>) {
            return [](const void*, Args... args) -> Ret {
                auto forward_args = std::forward_as_tuple(std::forward<Args>(args)...);
                return static_cast<Ret>(std::invoke(Func,
                    std::forward<type_list_element_t<Index, args_list>>(std::get<Index>(forward_args))...));
            };
        } else {
            ECS_ASSERT("unsupport bind function, maybe a bug", false);
            return nullptr;
        }
    }

    template <auto Func, typename Payload, size_t... Index>
    delegate_pointer_type wrap(Payload& payload, std::index_sequence<Index...>) {
        payload_ = &payload;
        using args_list = type_list<Args...>;
        if constexpr (std::is_invocable_r_v<Ret, decltype(Func), Payload&, type_list_element_t<Index, args_list>...>) {
            return [](const void* instance, Args... args) -> Ret {
                auto forward_args = std::forward_as_tuple(std::forward<Args>(args)...);
                return static_cast<Ret>(std::invoke(Func, *static_cast<Payload*>(const_cast<constness_as_t<void, Payload>*>(instance)),
                    std::forward<type_list_element_t<Index, args_list>>(std::get<Index>(forward_args))...));
            };
        } else {
            ECS_ASSERT("unsupport bind function, maybe a bug", false);
            return nullptr;
        }
    }

    template <auto Func, typename Payload, size_t... Index>
    delegate_pointer_type wrap(Payload* payload, std::index_sequence<Index...>) {
        payload_ = payload;
        using args_list = type_list<Args...>;
        if constexpr (std::is_invocable_r_v<Ret, decltype(Func), Payload*, type_list_element_t<Index, args_list>...>) {
            return [](const void* instance, Args... args) -> Ret {
                auto forward_args = std::forward_as_tuple(std::forward<Args>(args)...);
                return static_cast<Ret>(std::invoke(Func, static_cast<Payload*>(const_cast<constness_as_t<void, Payload>*>(instance)),
                    std::forward<type_list_element_t<Index, args_list>>(std::get<Index>(forward_args))...));
            };
        } else {
            ECS_ASSERT("unsupport bind function, maybe a bug", false);
            return nullptr;
        }
    }   
};

}