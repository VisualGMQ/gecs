#pragma once

#include "sigh_mixin.hpp"
#include "storage.hpp"
#include "querier.hpp"
#include "commands.hpp"
#include "resource.hpp"
#include "event_dispatcher.hpp"
#include "gecs/core/ident.hpp"
#include "gecs/core/utility.hpp"
#include "gecs/core/type_list.hpp"
#include "gecs/signal/sink.hpp"

#include <algorithm>
#include <memory>
#include <unordered_map>

namespace gecs {

namespace internal {

template <typename SparseSetT, typename Type>
struct storage_for;

template <typename EntityT, size_t PageSize, typename Type>
struct storage_for<basic_sparse_set<EntityT, PageSize>, Type> {
    using type = sigh_mixin<basic_storage<EntityT, Type, PageSize, std::allocator<Type>>>;
};

template <typename SparseSetT, typename Type>
using storage_for_t = typename storage_for<SparseSetT, Type>::type;

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

template<typename T>
struct is_resource<resource<T>> {
    static constexpr bool value = true;
};

template<typename T>
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
struct querier_construct_helper<WorldT, basic_querier<EntityT, PageSize, WorldT, Ts...>> {
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

}

template <typename EntityT, size_t PageSize>
class basic_world final {
public:
    using pool_base_type = basic_sparse_set<EntityT, PageSize>;

    template <typename Type>
    using storage_for_t = internal::storage_for_t<pool_base_type, Type>;

    using self_type = basic_world<EntityT, PageSize>;
    using entities_container_type = sigh_mixin<basic_storage<EntityT, EntityT, PageSize, void>>;
    using pool_container_type = std::vector<std::unique_ptr<pool_base_type>>;
    using pool_container_reference = pool_container_type&;
    using entity_type = EntityT;
    using system_type = void(*)(self_type&);
    using system_container_type = std::vector<system_type>; 

    static constexpr size_t page_size = PageSize;

    template <typename... Types>
    using querier_type = basic_querier<EntityT, PageSize, self_type, Types...>;

    template <typename T>
    using event_dispatcher_type = basic_event_dispatcher<T, self_type>;

    using commands_type = basic_commands<self_type>;

    template<typename... Ts>
    using system_t = void(Ts...);

    template <typename Type>
    struct storage_for_by_mutable {
        using type = std::conditional_t<internal::is_mutable_v<Type>, storage_for_t<internal::remove_mut_t<Type>>, const storage_for_t<internal::remove_mut_t<Type>>>;
    };

    template <typename Type>
    using storage_for_by_mutable_t = typename storage_for_by_mutable<Type>::type;

    entity_type create() noexcept {
        auto entity = entities_.emplace();
        return entity;
    }

    void destroy(EntityT entity) noexcept {
        if (entities_.contain(entity)) {
            entities_.remove(entity);

            for (auto& pool : pools_) {
                if (pool && pool->contain(entity)) {
                    pool->remove(entity);
                }
            }
        }
    }

    bool alive(EntityT entity) const noexcept {
        return entities_.contain(entity);
    }

    template <typename Type, typename... Args>
    Type& emplace(EntityT entity, Args&&... args) noexcept {
        return assure<Type>().emplace(entity, std::forward<Args>(args)...);
    }

    template <typename Type, typename... Args>
    Type& replace(EntityT entity, Args&&... args) noexcept {
        return assure<Type>().replace(entity, std::forward<Args>(args)...);
    }

    template <typename Type>
    const Type& get(EntityT entity) const noexcept {
        auto id = component_id_generator::gen<Type>();
        return static_cast<storage_for_t<Type>&>(*pools_[id])[entity];
    }

    template <typename Type>
    Type& get_mut(EntityT entity) noexcept {
        return const_cast<Type&>(std::as_const(*this).get());
    }

    template <typename Type>
    void remove(EntityT entity) noexcept {
        assure<Type>().remove(entity);
    }

    template <typename Type>
    bool contain(EntityT entity) const noexcept {
        auto id = component_id_generator::gen<Type>();
        if (id >= pools_.size()) {
            return false;
        } else {
            return pools_[id]->contain(entity);
        }
    }

    template <typename Type>
    const storage_for_t<Type>& pool() const noexcept {
        return assure<Type>();
    }

    template <typename Type>
    storage_for_t<Type>& pool() noexcept {
        return const_cast<storage_for_t<Type>>(std::as_const(*this).pool());
    }

    const entities_container_type& entities() const noexcept {
        return entities_;
    }

    entities_container_type& entities() noexcept {
        return const_cast<entities_container_type>(std::as_const(*this).entities());
    }

    typename entities_container_type::size_type size() const noexcept {
        return entities_.size();
    }

    template <typename Type>
    auto& pool() const noexcept {
        return assure<Type>();
    }

    template <typename... Types>
    querier_type<Types...> query() noexcept {
        static_assert(sizeof...(Types) > 0, "must provide query component");
        if constexpr (sizeof...(Types) == 1) {
            auto pool_tuple = std::tuple(&static_cast<storage_for_by_mutable_t<Types>&>(assure<internal::remove_mut_t<Types>>())...);
            return querier_type<Types...>(pool_tuple, std::get<0>(pool_tuple)->packed());
        } else {
            typename pool_base_type::packed_container_type entities;
            std::array indices = { component_id_generator::gen<internal::remove_mut_t<Types>>()... };

            bool valid_query = std::all_of(indices.begin(), indices.end(),
                [&](auto idx) {
                    return idx < pools_.size() && pools_[idx] != nullptr;
                });

            if (valid_query) {
                size_t idx = minimal_idx<Types...>(pools_, indices);
                for (int i = 0; i < pools_[idx]->size(); i++) {
                    auto entity = pools_[idx]->packed()[i];
                    bool is_contain_all = true;
                    for (auto index : indices) {
                        if (!pools_[index]->contain(static_cast<entity_type>(entity))) {
                            is_contain_all = false;
                            break;
                        }
                    }
                    if (is_contain_all) {
                        entities.push_back(entity);
                    }
                }
            }
            return querier_type<Types...>(std::tuple(&static_cast<storage_for_by_mutable_t<Types>&>(assure<internal::remove_mut_t<Types>>())...), entities);
        }
    }

    commands_type commands() noexcept {
        return commands_type{*this};
    }

    template <typename T>
    resource<T> res() noexcept {
        return resource<T>{};
    }

    template <typename T>
    event_dispatcher_type<T> event_dispatcher() noexcept {
        auto dispatcher = event_dispatcher_type<T>{*this};
        auto id = dispatcher_id_generator::gen<T>();
        if (auto it = auto_dispatch_fns_.find(id); it == auto_dispatch_fns_.end()) {
            auto_dispatch_fns_.emplace(id,
            +[](self_type& self){
                event_dispatcher_type<T>(self).update();
            });
        }
        return dispatcher;
    }

    template <typename Type>
    storage_for_t<Type>& assure() noexcept {
        size_t idx = component_id_generator::gen<Type>();
        if (idx >= pools_.size()) {
            pools_.resize(idx + 1);
        }
        if (pools_[idx] == nullptr) {
            pools_[idx] = std::make_unique<storage_for_t<Type>>();
        }
        return static_cast<storage_for_t<Type>&>(*pools_[idx]);
    }

    template <auto System>
    auto& regist_startup_system() noexcept {
        startup_systems_.emplace_back([](self_type& world) {
            using type_list = typename system_traits<strip_function_pointer_to_type_t<std::decay_t<decltype(System)>>>::types;
            invoke_arbitary_param_system<System, type_list>(world, std::make_index_sequence<type_list::size>{});
        });
        return *this;
    }

    template <auto System>
    auto& regist_update_system() noexcept {
        update_systems_.emplace_back([](self_type& world) {
            using type_list = typename system_traits<strip_function_pointer_to_type_t<std::decay_t<decltype(System)>>>::types;
            invoke_arbitary_param_system<System, type_list>(world, std::make_index_sequence<type_list::size>{});
        });
        return *this;
    }

    template <auto System>
    auto& regist_shutdown_system() noexcept {
        shutdown_systems_.emplace_back([](self_type& world) {
            using type_list = typename system_traits<strip_function_pointer_to_type_t<std::decay_t<decltype(System)>>>::types;
            invoke_arbitary_param_system<System, type_list>(world, std::make_index_sequence<type_list::size>{});
        });
        return *this;
    }

    void startup() noexcept {
        for (auto sys : startup_systems_) {
            sys(*this);
        }
    }

    void update() noexcept {
        for (auto sys : update_systems_) {
            sys(*this);
        }
        for (auto [key, fn] : auto_dispatch_fns_) {
            fn(*this);
        }
    }

    void shutdown() noexcept {
        for (auto sys : shutdown_systems_) {
            sys(*this);
        }
    }

    template <typename T>
    auto on_construct() noexcept {
        if constexpr (std::is_same_v<T, entity_type>) {
            return sink(entities_.on_construct());
        } else {
            return sink(assure<T>().on_construct());
        }
    }

    template <typename T>
    auto on_destruction() noexcept {
        if constexpr (std::is_same_v<T, entity_type>) {
            return sink(entities_.on_destruct());
        } else {
            return sink(assure<T>().on_destruct());
        }
    }

    template <typename T>
    auto on_update() noexcept {
        if constexpr (std::is_same_v<T, entity_type>) {
            ECS_ASSERT("entity don't has update listener", false);
            return internal::null_entity_t{};
        } else {
            return sink(assure<T>().on_update());
        }
    }

private:
    using event_auto_dispatch_fn_pointer = void(*)(self_type&);
    using auto_dispatch_fn_container = std::unordered_map<dispatcher_id_generator::value_type, event_auto_dispatch_fn_pointer>;

    pool_container_type pools_;
    entities_container_type entities_;
    system_container_type startup_systems_;
    system_container_type update_systems_;
    system_container_type shutdown_systems_;;
    auto_dispatch_fn_container auto_dispatch_fns_;

    template <typename T>
    struct system_traits;
    
    template <typename... Ts>
    struct system_traits<system_t<Ts...>> {
        using types = type_list<Ts...>;
    };

    template <auto System, typename List, size_t... Idx>
    static void invoke_arbitary_param_system(self_type& world, std::index_sequence<Idx...>) {
        std::invoke(System, internal::construct<self_type, type_list_element_t<Idx, List>>(world)...);
    }

    template <typename... Types>
    size_t minimal_idx(pool_container_reference& pools, const std::array<size_t, sizeof...(Types)>& indices) {
        size_t minimal = std::numeric_limits<size_t>::max();
        size_t min_idx = 0;

        for (auto idx : indices) {
            minimal = pools[idx]->size() < minimal ? pools[idx]->size() : minimal;
            min_idx = idx;
        }

        return min_idx;
    }
};

}
