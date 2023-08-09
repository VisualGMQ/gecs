#pragma once

#include "gecs/utilities/utility.hpp"

namespace gecs {

template <typename Func>
class Delegate;

template <typename Ret, typename... Args>
class Delegate<Ret(Args...)> final {
public:
    using DelegateType = Ret(*)(const void*, Args...);

    template <auto Func>
    void Connect() noexcept {
        fn_ = wrap<Func>(std::make_index_sequence<sizeof...(Args)>{});
    }

    void Connect(DelegateType d) {
        payload_ = nullptr;
        fn_ = d;
    }

    template <typename Payload>
    void Connect(DelegateType d, Payload& payload) {
        payload_ = &payload;
        fn_ = d;
    }

    template <typename Payload>
    void Connect(DelegateType d, Payload* payload) {
        payload_ = payload;
        fn_ = d;
    }

    template <auto Func, typename Payload>
    void Connect(Payload& payload) {
        fn_ = wrap<Func>(payload, std::make_index_sequence<sizeof...(Args)>{});
    }

    template <auto Func, typename Payload>
    void Connect(Payload* payload) {
        fn_ = wrap<Func>(payload, std::make_index_sequence<sizeof...(Args)>{});
    }

    template <auto Func, size_t... Index>
    void Connect(std::index_sequence<Index...> indices) noexcept {
        fn_ = wrap<Func>(indices);
    }

    template <auto Func, typename Payload, size_t... Index>
    void Connect(Payload& payload, std::index_sequence<Index...> indices) {
        fn_ = wrap<Func>(payload, indices);
    }

    template <auto Func, typename Payload, size_t... Index>
    void Connect(Payload* payload, std::index_sequence<Index...> indices) {
        fn_ = wrap<Func>(payload, indices);
    }

    Ret operator()(Args... args) {
        return std::invoke(fn_, payload_, std::forward<Args>(args)...);
    }

    void Reset() {
        payload_ = nullptr;
        fn_ = nullptr;
    }

    operator bool() const {
        return fn_ != nullptr;
    }

    void Release() {
        payload_ = nullptr;
        fn_ = nullptr;
    }

    bool operator==(const Delegate& o) const {
        return o.payload_ == payload_ && o.fn_ == fn_;
    }

    bool operator!=(const Delegate& o) const {
        return !(*this == o);
    }

    ~Delegate() {
        Release();
    }

private:
    const void* payload_ = nullptr;
    DelegateType fn_ = nullptr; 

    template <auto Func, size_t... Index>
    DelegateType wrap(std::index_sequence<Index...>) {
        payload_ = nullptr;
        using args_list = type_list<Args...>;
        if constexpr (std::is_invocable_r_v<Ret, decltype(Func), type_list_elements_t<Index, args_list>...>) {
            return [](const void*, Args... args) -> Ret {
                auto forward_args = std::forward_as_tuple(std::forward<Args>(args)...);
                return static_cast<Ret>(std::invoke(Func,
                    std::forward<type_list_elements_t<Index, args_list>>(std::get<Index>(forward_args))...));
            };
        } else {
            static_assert(false, "unsupport bind function, maybe a bug");
            return nullptr;
        }
    }

    template <auto Func, typename Payload, size_t... Index>
    DelegateType wrap(Payload& payload, std::index_sequence<Index...>) {
        payload_ = &payload;
        using args_list = type_list<Args...>;
        if constexpr (std::is_invocable_r_v<Ret, decltype(Func), Payload&, type_list_elements_t<Index, args_list>...>) {
            return [](const void* instance, Args... args) -> Ret {
                auto forward_args = std::forward_as_tuple(std::forward<Args>(args)...);
                return static_cast<Ret>(std::invoke(Func, *static_cast<Payload*>(const_cast<constness_as_t<void, Payload>*>(instance)),
                    std::forward<type_list_elements_t<Index, args_list>>(std::get<Index>(forward_args))...));
            };
        } else {
            static_assert(false, "unsupport bind function, maybe a bug");
            return nullptr;
        }
    }

    template <auto Func, typename Payload, size_t... Index>
    DelegateType wrap(Payload* payload, std::index_sequence<Index...>) {
        payload_ = payload;
        using args_list = type_list<Args...>;
        if constexpr (std::is_invocable_r_v<Ret, decltype(Func), Payload*, type_list_elements_t<Index, args_list>...>) {
            return [](const void* instance, Args... args) -> Ret {
                auto forward_args = std::forward_as_tuple(std::forward<Args>(args)...);
                return static_cast<Ret>(std::invoke(Func, static_cast<Payload*>(const_cast<constness_as_t<void, Payload>*>(instance)),
                    std::forward<type_list_elements_t<Index, args_list>>(std::get<Index>(forward_args))...));
            };
        } else {
            static_assert(false, "unsupport bind function, maybe a bug");
            return nullptr;
        }
    }   
};

}