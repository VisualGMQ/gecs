#pragma once

#include "gecs/entity/commands.hpp"
#include "gecs/entity/event_dispatcher.hpp"
#include "gecs/core/utility.hpp"
#include "gecs/entity/querier.hpp"
#include "gecs/entity/resource.hpp"
#include "gecs/entity/registry_wrapper.hpp"

#include <functional>

namespace gecs {

// fwd declare
template <typename T, typename WorldT>
class basic_event_dispatcher;

template <typename T, typename WorldT>
class basic_event_dispatcher_wrapper;

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

template <typename T, typename WorldT>
struct is_event_dispatcher<basic_event_dispatcher_wrapper<T, WorldT>> {
    static constexpr bool value = true;
};

template <typename T>
constexpr bool is_event_dispatcher_v = is_event_dispatcher<T>::value;

template <typename T>
struct is_registry {
    static constexpr bool value = false;
};

template <typename T>
struct is_registry<registry_wrapper<T>> {
    static constexpr bool value = true;
};

template <typename T>
constexpr bool is_registry_v = is_registry<T>::value;


template <typename RegistryT, typename Querier>
struct querier_construct_helper;

template <typename EntityT, size_t PageSize, typename RegistryT, typename... Ts>
struct querier_construct_helper<
    RegistryT, basic_querier<EntityT, PageSize, RegistryT, Ts...>> {
    using querier_type = basic_querier<EntityT, PageSize, RegistryT, Ts...>;

    querier_type operator()(RegistryT& registry) const {
        return registry.template query<Ts...>();
    }
};

template <typename T, typename RegistryT>
T construct_resource(RegistryT& reg) noexcept {
    return reg.template res<typename T::resource_type>();
}

template <typename T, typename RegistryT>
T construct_event_dispatcher(RegistryT& reg) {
    return reg.template event_dispatcher<typename T::event_type>();
}

template <typename RegistryT>
auto construct_registry(RegistryT& reg) {
    return registry_wrapper{reg};
}

template <typename Querier, typename RegistryT>
auto construct_querier(RegistryT& registry) {
    return querier_construct_helper<RegistryT, Querier>{}(registry);
}

template <typename RegistryT>
typename RegistryT::commands_type construct_commands(RegistryT& registry) {
    return registry.commands();
}

template <typename RegistryT, typename T>
auto construct(RegistryT& reg) {
    if constexpr (is_querier_v<T>) {
        return construct_querier<T>(reg);
    } else if constexpr (is_commands_v<T>) {
        return construct_commands(reg);
    } else if constexpr (is_resource_v<T>) {
        return construct_resource<T>(reg);
    } else if constexpr (is_event_dispatcher_v<T>) {
        return construct_event_dispatcher<T>(reg);
    } else if constexpr (is_registry_v<T>) {
        return construct_registry(reg);
    } else {
        GECS_ASSERT(false, "can't construct a unsupport type");
    }
}

template <typename WorldT, typename... Ts>
auto construct_by_types(WorldT& world) {
    return std::make_tuple(construct<WorldT, Ts>...);
}

template <typename WorldT, typename Types, size_t... Idx>
auto construct_by_types(WorldT& world, std::index_sequence<Idx...>) {
    return std::make_tuple(construct<list_element_t<Types, Idx>>...);
}

template <typename T>
struct system_traits;

template <typename... Ts>
struct system_traits<void(Ts...)> {
    using types = type_list<Ts...>;
};

template <typename RegistryT, typename... Ts>
class system_constructor final {
private:
    template <auto Func, typename List, size_t... Idx>
    static constexpr void invoke_arbitary_param_system(
	Ts... params,
        RegistryT& reg, std::index_sequence<Idx...>) {
        std::invoke(Func, std::forward<Ts>(params)...,
                    internal::template construct<
                        RegistryT, list_element_t<List, Idx>>(reg)...);
    }

public:
    using function_type = void (*)(Ts..., RegistryT&);

    template <auto Func>
    static constexpr function_type value = [](Ts... params, RegistryT& reg) {
        using arg_list =
            typename system_traits<strip_function_pointer_to_type_t<
                std::decay_t<decltype(Func)>>>::types;
        system_constructor<RegistryT, Ts...>::template invoke_arbitary_param_system<
            Func, arg_list>(std::forward<Ts>(params)..., reg, make_index_range<sizeof...(Ts), arg_list::size>{});
    };

    template <auto Func>
    static constexpr function_type construct() {
        return [](Ts... params, RegistryT& reg) {
            using type_list =
                typename system_traits<strip_function_pointer_to_type_t<
                    std::decay_t<decltype(Func)>>>::types;
            invoke_arbitary_param_system<Func, type_list>(std::forward<Ts>(params)...,
                reg, make_index_range<sizeof...(Ts), type_list::size>{});
        };
    }
};

template <auto Func, typename RegistryT, typename... Ts>
constexpr auto system_constructor_v =
    system_constructor<RegistryT, Ts...>::template value<Func>;

}  // namespace internal

}  // namespace gecs
