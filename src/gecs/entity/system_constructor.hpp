#pragma once

#include "commands.hpp"
#include "event_dispatcher.hpp"
#include "gecs/core/utility.hpp"
#include "querier.hpp"
#include "resource.hpp"

namespace gecs {

// fwd declare
template <typename T, typename WorldT>
class basic_event_dispatcher;

namespace internal {

template <typename T>
struct is_querier {
    static constexpr bool value = false;
};

template <typename EntityT, size_t offset, typename WorldT, typename... Ts>
struct is_querier<basic_querier<EntityT, offset, WorldT, Ts...>> {
    static constexpr bool value = true;
};

template <typename T>
constexpr bool is_querier_v = is_querier<T>::value;

template <typename T>
struct is_commands {
    static constexpr bool value = false;
};

template <typename WorldT>
struct is_commands<basic_commands<WorldT>> {
    static constexpr bool value = true;
};

template <typename T>
constexpr bool is_commands_v = is_commands<T>::value;

template <typename T>
struct is_resource {
    static constexpr bool value = false;
};

template <typename T>
struct is_resource<resource<T>> {
    static constexpr bool value = true;
};

template <typename T>
constexpr bool is_resource_v = is_resource<T>::value;

template <typename T>
struct is_event_dispatcher {
    static constexpr bool value = false;
};

template <typename T, typename WorldT>
struct is_event_dispatcher<basic_event_dispatcher<T, WorldT>> {
    static constexpr bool value = true;
};

template <typename T>
constexpr bool is_event_dispatcher_v = is_event_dispatcher<T>::value;

template <typename WorldT, typename Querier>
struct querier_construct_helper;

template <typename EntityT, size_t PageSize, typename WorldT, typename... Ts>
struct querier_construct_helper<
    WorldT, basic_querier<EntityT, PageSize, WorldT, Ts...>> {
    using querier_type = basic_querier<EntityT, PageSize, WorldT, Ts...>;

    querier_type operator()(WorldT& world) const {
        return world.template query<Ts...>();
    }
};

template <typename T, typename WorldT>
T construct_resource(WorldT& world) noexcept {
    return world.template res<typename T::resource_type>();
}

template <typename T, typename WorldT>
T construct_event_dispatcher(WorldT& world) {
    return world.template event_dispatcher<typename T::event_type>();
}

template <typename Querier, typename WorldT>
auto construct_querier(WorldT& world) {
    return querier_construct_helper<WorldT, Querier>{}(world);
}

template <typename WorldT>
typename WorldT::commands_type construct_commands(WorldT& world) {
    return world.commands();
}

template <typename WorldT, typename T>
auto construct(WorldT& world) {
    if constexpr (is_querier_v<T>) {
        return construct_querier<T>(world);
    } else if constexpr (is_commands_v<T>) {
        return construct_commands<WorldT>(world);
    } else if constexpr (is_resource_v<T>) {
        return construct_resource<T>(world);
    } else if constexpr (is_event_dispatcher_v<T>) {
        return construct_event_dispatcher<T>(world);
    } else {
        ECS_ASSERT("can't construct a unsupport type", false);
    }
}

template <typename WorldT, typename... Ts>
auto construct_by_types(WorldT& world) {
    return std::make_tuple(construct<Ts>...);
}

template <typename WorldT, typename Types, size_t... Idx>
auto construct_by_types(WorldT& world, std::index_sequence<Idx...>) {
    return std::make_tuple(construct<type_list_element_t<Idx, Types>>...);
}

template <typename T>
struct system_traits;

template <typename... Ts>
struct system_traits<void(Ts...)> {
    using types = type_list<Ts...>;
};

template <typename WorldT, typename... Ts>
class system_constructor final {
public:
    using function_type = void (*)(Ts..., WorldT&);

    template <auto Func>
    static constexpr function_type value = [](Ts... params, WorldT& world) {
        using arg_list =
            typename system_traits<strip_function_pointer_to_type_t<
                std::decay_t<decltype(Func)>>>::types;
        system_constructor<WorldT, Ts...>::invoke_arbitary_param_system<
            Func, arg_list>(std::forward<Ts>(params)..., world,
                            make_index_range<sizeof...(Ts), arg_list::size>{});
    };

    template <auto Func>
    static constexpr function_type construct() {
        return [](Ts... params, WorldT& world) {
            using type_list =
                typename system_traits<strip_function_pointer_to_type_t<
                    std::decay_t<decltype(Func)>>>::types;
            invoke_arbitary_param_system<Func, type_list>(
                std::forward<Ts>(params)..., world,
                make_index_range<sizeof...(Ts), type_list::size>{});
        };
    }

private:
    template <auto Func, typename List, size_t... Idx>
    static constexpr void invoke_arbitary_param_system(
        Ts... params, WorldT& world, std::index_sequence<Idx...>) {
        std::invoke(Func, std::forward<Ts>(params)...,
                    internal::template construct<
                        WorldT, type_list_element_t<Idx, List>>(world)...);
    }
};

template <auto Func, typename WorldT, typename... Ts>
constexpr auto system_constructor_v =
    system_constructor<WorldT, Ts...>::value<Func>;

}  // namespace internal

}  // namespace gecs