#pragma once

#include "storage.hpp"
#include "gecs/core/ident.hpp"
#include "gecs/entity/querier.hpp"

namespace gecs {

namespace internal {

template <typename SparseSetT, typename Type>
struct storage_for;

template <typename EntityT, size_t PageSize, typename Type>
struct storage_for<basic_sparse_set<EntityT, PageSize>, Type> {
    using type = basic_storage<EntityT, Type, PageSize, std::allocator<Type>>;
};

template <typename SparseSetT, typename Type>
using storage_for_t = typename storage_for<SparseSetT, Type>::type;

}

template <typename EntityT, size_t PageSize>
class basic_world final {
public:
    using self_type = basic_world<EntityT, PageSize>;
    using entities_container = basic_storage<EntityT, EntityT, PageSize, void>;
    using pool_base_type = basic_sparse_set<EntityT, PageSize>;

    template <typename Type>
    using storage_for_t = internal::storage_for_t<pool_base_type, Type>;

    template <typename Type>
    struct storage_for_by_mutable {
        using type = std::conditional_t<internal::is_mutable_v<Type>, storage_for_t<internal::remove_mut_t<Type>>, const storage_for_t<internal::remove_mut_t<Type>>>;
    };

    template <typename Type>
    using storage_for_by_mutable_t = typename storage_for_by_mutable<Type>::type;

    EntityT create() noexcept {
        return entities_.emplace();
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
        auto id = id_generator::gen<Type>();
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
        auto id = id_generator::gen<Type>();
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

    const entities_container& entities() const noexcept {
        return entities_;
    }

    entities_container& entities() noexcept {
        return const_cast<entities_container>(std::as_const(*this).entities());
    }

    typename entities_container::size_type size() const noexcept {
        return entities_.size();
    }

    template <typename Type>
    auto& pool() const noexcept {
        return assure<Type>();
    }

    template <typename Type>
    auto query() noexcept {
        auto& pool = assure<internal::remove_mut_t<Type>>();

        return basic_querier<EntityT, PageSize, self_type, Type>(std::tuple(&static_cast<storage_for_by_mutable_t<Type>&>(pool)));
    }

private:
    std::vector<std::unique_ptr<pool_base_type>> pools_;
    entities_container entities_;

    template <typename Type>
    storage_for_t<Type>& assure() noexcept {
        size_t idx = id_generator::gen<Type>();
        if (idx >= pools_.size()) {
            pools_.resize(idx + 1);
        }
        if (pools_[idx] == nullptr) {
            pools_[idx] = std::make_unique<storage_for_t<Type>>();
        }
        return static_cast<storage_for_t<Type>&>(*pools_[idx]);
    }
};

}