#pragma once

#include "commands.hpp"
#include "event_dispatcher.hpp"
#include "gecs/core/ident.hpp"
#include "gecs/core/type_list.hpp"
#include "gecs/core/utility.hpp"
#include "gecs/signal/sink.hpp"
#include "querier.hpp"
#include "resource.hpp"
#include "sigh_mixin.hpp"
#include "storage.hpp"
#include "system_constructor.hpp"

#include <algorithm>
#include <memory>
#include <optional>
#include <unordered_map>

namespace gecs {

namespace internal {

template <typename SparseSetT, typename Type>
struct storage_for;

template <typename EntityT, size_t PageSize, typename Type>
struct storage_for<basic_sparse_set<EntityT, PageSize>, Type> {
    using type = sigh_mixin<
        basic_storage<EntityT, Type, PageSize, std::allocator<Type>>>;
};

template <typename SparseSetT, typename Type>
using storage_for_t = typename storage_for<SparseSetT, Type>::type;

}  // namespace internal

template <typename WorldT, typename EntityT, size_t PageSize>
class basic_registry final {
public:
    using self_type = basic_registry<WorldT, EntityT, PageSize>;
    using pool_base_type = basic_sparse_set<EntityT, PageSize>;
    using owner_type = WorldT;

    template <typename Type>
    using storage_for_t = internal::storage_for_t<pool_base_type, Type>;

    using entities_container_type =
        sigh_mixin<basic_storage<EntityT, EntityT, PageSize, void>>;
    using pool_container_type = std::vector<std::unique_ptr<pool_base_type>>;
    using pool_container_reference = pool_container_type&;
    using entity_type = EntityT;
    using system_type = void (*)(self_type&);
    using system_container_type = std::vector<system_type>;
    using event_dispatcher_container = std::vector<std::unique_ptr<event_dispatcher_base>>;

    static constexpr size_t page_size = PageSize;

    template <typename... Types>
    using querier_type = basic_querier<EntityT, PageSize, self_type, Types...>;

    template <typename T>
    using event_dispatcher_type = basic_event_dispatcher<T, self_type>;

    template <typename T>
    using event_dispatcher_wrapper_type = basic_event_dispatcher_wrapper<T, self_type>;

    using commands_type = basic_commands<owner_type>;

    template <typename Type>
    struct storage_for_by_mutable {
        using type = std::conditional_t<
            internal::is_mutable_v<Type>,
            storage_for_t<internal::remove_mut_t<Type>>,
            const storage_for_t<internal::remove_mut_t<Type>>>;
    };

    template <typename Type>
    using storage_for_by_mutable_t =
        typename storage_for_by_mutable<Type>::type;

    template <typename T>
    using event_dispatcher_type_for_t = event_dispatcher_type_for_t<T, self_type>;

    basic_registry(owner_type& owner): owner_(&owner) {}

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
        return assure_storage<Type>().emplace(entity, std::forward<Args>(args)...);
    }

    template <typename Type, typename... Args>
    Type& replace(EntityT entity, Args&&... args) noexcept {
        return assure_storage<Type>().replace(entity, std::forward<Args>(args)...);
    }

    template <typename Type>
    const Type& get(EntityT entity) const noexcept {
        auto id = component_id_generator::gen<Type>();
        return static_cast<storage_for_t<Type>&>(*pools_[id])[entity];
    }

    GECS_REFERENCE_ANY get_mut(EntityT entity, const void* type_info) noexcept {
        for (auto& info : type_infos_) {
            if (info.type_info == type_info) {
                return info.convert_to_any(*this, entity);
            }
        }
        return {};
    }

    template <typename Type>
    Type& get_mut(EntityT entity) noexcept {
        return const_cast<Type&>(std::as_const(*this).template get<Type>(entity));
    }

    template <typename Type>
    void remove(EntityT entity) noexcept {
        assure_storage<Type>().remove(entity);
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
        return assure_storage<Type>();
    }

    template <typename Type>
    storage_for_t<Type>& pool() noexcept {
        return const_cast<storage_for_t<Type>>(std::as_const(*this).pool());
    }

    template <typename Type>
    bool has(entity_type entity) const noexcept {
        auto& p = pool<Type>();
        return p.contain(entity);
    }

    bool has(entity_type entity, const void* type_info) const noexcept {
        for (int i = 0; i < pools_.size(); i++) {
            if (pools_[i]->contain(entity) &&
                type_infos_[i].type_info == type_info) {
                return true;
            }
        }
        return false;
    }

    const entities_container_type& entities() const noexcept {
        return entities_;
    }

    entities_container_type& entities() noexcept {
        return const_cast<entities_container_type&>(
            std::as_const(*this).entities());
    }

    typename entities_container_type::size_type size() const noexcept {
        return entities_.size();
    }

    template <typename Type>
    auto& pool() const noexcept {
        return assure_storage<Type>();
    }

    template <typename... Types>
    querier_type<Types...> query() noexcept {
        static_assert(sizeof...(Types) > 0, "must provide query component");
        if constexpr (sizeof...(Types) == 1) {
            auto pool_tuple =
                std::tuple(&static_cast<storage_for_by_mutable_t<Types>&>(
                    assure_storage<internal::remove_mut_t<Types>>())...);
            return querier_type<Types...>(pool_tuple,
                                          std::get<0>(pool_tuple)->packed());
        } else {
            typename pool_base_type::packed_container_type entities;
            std::array indices = {component_id_generator::gen<
                internal::remove_mut_t<Types>>()...};

            bool valid_query =
                std::all_of(indices.begin(), indices.end(), [&](auto idx) {
                    return idx < pools_.size() && pools_[idx] != nullptr;
                });

            if (valid_query) {
                size_t idx = minimal_idx<Types...>(pools_, indices);
                for (int i = 0; i < pools_[idx]->size(); i++) {
                    auto entity = pools_[idx]->packed()[i];
                    bool is_contain_all = true;
                    for (auto index : indices) {
                        if (!pools_[index]->contain(
                                static_cast<entity_type>(entity))) {
                            is_contain_all = false;
                            break;
                        }
                    }
                    if (is_contain_all) {
                        entities.push_back(entity);
                    }
                }
            }
            return querier_type<Types...>(
                std::tuple(&static_cast<storage_for_by_mutable_t<Types>&>(
                    assure_storage<internal::remove_mut_t<Types>>())...),
                entities);
        }
    }

    commands_type commands() noexcept { return commands_type{*owner_, *this}; }

    template <typename T>
    resource<T> res() noexcept {
        return resource<T>{};
    }

    template <typename T>
    event_dispatcher_wrapper_type<T> event_dispatcher() noexcept {
        return event_dispatcher_wrapper_type<T>(assure_dispatcher<T>());
    }

    template <typename Type>
    storage_for_t<Type>& assure_storage() noexcept {
        size_t idx = component_id_generator::gen<Type>();
        if (idx >= pools_.size()) {
            pools_.resize(idx + 1);
            type_infos_.resize(idx + 1);
        }
        if (pools_[idx] == nullptr) {
            pools_[idx] =
                std::make_unique<storage_for_t<Type>>(GECS_GET_TYPE_INFO(Type));
            type_infos_[idx].type_info = GECS_GET_TYPE_INFO(Type);
            type_infos_[idx].convert_to_any =
                TypeInfo::template convert_type_to_any<Type>;
        }
        return static_cast<storage_for_t<Type>&>(*pools_[idx]);
    }

    template <typename Type>
    event_dispatcher_type_for_t<Type>& assure_dispatcher() noexcept {
        size_t idx = dispatcher_id_generator::gen<Type>();
        if (idx >= event_dispatchers_.size()) {
            event_dispatchers_.resize(idx + 1);
        }
        if (event_dispatchers_[idx] == nullptr) {
            event_dispatchers_[idx] =
                std::make_unique<event_dispatcher_type_for_t<Type>>(*this);
        }
        return static_cast<event_dispatcher_type_for_t<Type>&>(
            *event_dispatchers_[idx]);
    }

    template <auto System>
    auto& regist_startup_system() noexcept {
        startup_systems_.emplace_back(internal::system_constructor<
                                      self_type>::template construct<System>());
        return *this;
    }

    template <auto System>
    auto& regist_update_system() noexcept {
        update_systems_.emplace_back(internal::system_constructor<
                                     self_type>::template construct<System>());
        return *this;
    }

    template <auto System>
    auto& regist_shutdown_system() noexcept {
        shutdown_systems_.emplace_back(
            internal::system_constructor<self_type>::template construct<
                System>());
        return *this;
    }

    template <typename T>
    auto& add_state(T value) {
        static_assert(
            std::is_enum_v<T> && std::is_integral_v<std::underlying_type_t<T>>,
            "state must be an enum");
        states_.emplace(static_cast<uint32_t>(value), State{});
        return *this;
    }

    template <auto System, typename T>
    auto& regist_enter_system_to_state(T value) noexcept {
        if (auto it =
                states_.find(static_cast<std::underlying_type_t<T>>(value));
            it != states_.end()) {
            it->second.on_enter.emplace_back(
                internal::system_constructor<self_type>::template construct<
                    System>());
        } else {
            GECS_ASSERT(false, "state not exists");
        }
        return *this;
    }

    template <auto System, typename T>
    auto& regist_update_system_to_state(T value) noexcept {
        if (auto it =
                states_.find(static_cast<std::underlying_type_t<T>>(value));
            it != states_.end()) {
            it->second.on_update.emplace_back(
                internal::system_constructor<self_type>::template construct<
                    System>());
        } else {
            GECS_ASSERT(false, "state not exists");
        }
        return *this;
    }

    template <auto System, typename T>
    auto& regist_exit_system_to_state(T value) noexcept {
        if (auto it =
                states_.find(static_cast<std::underlying_type_t<T>>(value));
            it != states_.end()) {
            it->second.on_exit.emplace_back(
                internal::system_constructor<self_type>::template construct<
                    System>());
        } else {
            GECS_ASSERT(false, "state not exists");
        }
        return *this;
    }

    template <typename T>
    void switch_state_immediatly(T state) {
        do_switch_state(static_cast<std::underlying_type_t<T>>(state));
    }

    template <typename T>
    void switch_state(T state) {
        will_change_state_ = static_cast<std::underlying_type_t<T>>(state);
    }

    void startup() noexcept {
        for (int i = 0; i < startup_systems_.size(); i++) {
            startup_systems_[i](*this);
        }

        if (cur_state_) {
            for (auto& enter : states_[cur_state_.value()].on_enter) {
                enter(*this);
            }
        }
    }

    void update() noexcept {
        for (auto sys : update_systems_) {
            sys(*this);
        }

        if (cur_state_) {
            for (auto& update : states_[cur_state_.value()].on_update) {
                update(*this);
            }
        }

        for (auto& dispatcher : event_dispatchers_) {
            dispatcher->trigger_cached();
            dispatcher->clear_cache();
        }

        if (will_change_state_) {
            do_switch_state(will_change_state_.value());
            will_change_state_.reset();
        }
    }

    void shutdown() noexcept {
        if (cur_state_) {
            for (auto& exit : states_[cur_state_.value()].on_exit) {
                exit(*this);
            }
        }

        for (auto sys : shutdown_systems_) {
            sys(*this);
        }
    }

    template <typename T>
    auto& start_with_state(T state) {
        cur_state_ = static_cast<std::underlying_type_t<T>>(state);
        return *this;
    }

    template <typename T>
    auto on_construct() noexcept {
        if constexpr (std::is_same_v<T, entity_type>) {
            return sink(entities_.on_construct());
        } else {
            return sink(assure_storage<T>().on_construct());
        }
    }

    template <typename T>
    auto on_destruction() noexcept {
        if constexpr (std::is_same_v<T, entity_type>) {
            return sink(entities_.on_destruct());
        } else {
            return sink(assure_storage<T>().on_destruct());
        }
    }

    template <typename T>
    auto on_update() noexcept {
        if constexpr (std::is_same_v<T, entity_type>) {
            GECS_ASSERT(false, "entity don't has update listener");
            return null_entity;
        } else {
            return sink(assure_storage<T>().on_update());
        }
    }

private:
    struct State final {
        system_container_type on_enter;
        system_container_type on_update;
        system_container_type on_exit;
    };

    std::unordered_map<uint32_t, State> states_;
    std::optional<uint32_t> cur_state_;
    std::optional<uint32_t> will_change_state_;

    struct TypeInfo final {
        const void* type_info;
        GECS_REFERENCE_ANY (*convert_to_any)(self_type&, entity_type);

        template <typename T>
        static GECS_REFERENCE_ANY convert_type_to_any(self_type& reg,
                                                      entity_type entity) {
            auto ref = GECS_REFERENCE_ANY{reg.get_mut<T>(entity)};
            return ref;
        }
    };

    using type_info_container_type = std::vector<TypeInfo>;

    owner_type* owner_;

    pool_container_type pools_;
    type_info_container_type type_infos_;

    entities_container_type entities_;
    system_container_type startup_systems_;
    system_container_type update_systems_;
    system_container_type shutdown_systems_;
    event_dispatcher_container event_dispatchers_;

    template <typename... Types>
    size_t minimal_idx(pool_container_reference& pools,
                       const std::array<size_t, sizeof...(Types)>& indices) {
        size_t minimal = std::numeric_limits<size_t>::max();
        size_t min_idx = 0;

        for (auto idx : indices) {
            minimal =
                pools[idx]->size() < minimal ? pools[idx]->size() : minimal;
            min_idx = idx;
        }

        return min_idx;
    }

    void do_switch_state(uint32_t state) {
        if (cur_state_) {
            for (auto& exit : states_[cur_state_.value()].on_exit) {
                exit(*this);
            }
        }

        cur_state_ = state;
        for (auto& enter : states_[cur_state_.value()].on_enter) {
            enter(*this);
        }
    }
};

}  // namespace gecs
