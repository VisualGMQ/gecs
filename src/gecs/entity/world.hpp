#pragma once

#include "storage.hpp"
#include "gecs/core/ident.hpp"

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
    using entities_container = basic_storage<EntityT, EntityT, PageSize, void>;
    using pool_base_type = basic_sparse_set<EntityT, PageSize>;

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
        auto id = id_generator::gen<Type>();
        return assure<Type>(id).emplace(entity, std::forward<Args>(args)...);
    }

    template <typename Type, typename... Args>
    Type& replace(EntityT entity, Args&&... args) noexcept {
        auto id = id_generator::gen<Type>();
        return assure<Type>(id).replace(entity, std::forward<Args>(args)...);
    }

    template <typename Type>
    const Type& get(EntityT entity) const noexcept {
        auto id = id_generator::gen<Type>();
        return static_cast<internal::storage_for_t<pool_base_type, Type>&>(*pools_[id])[entity];
    }

    template <typename Type>
    Type& get_mut(EntityT entity) noexcept {
        return const_cast<Type&>(std::as_const(*this).get());
    }

    template <typename Type>
    void remove(EntityT entity) noexcept {
        auto id = id_generator::gen<Type>();
        assure<Type>(id).remove(entity);
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
    const internal::storage_for_t<pool_base_type, Type>& pool() const noexcept {
        return assure<Type>(id_generator::gen<Type>());
    }

    template <typename Type>
    internal::storage_for_t<pool_base_type, Type>& pool() noexcept {
        return const_cast<internal::storage_for_t<pool_base_type, Type>>(std::as_const(*this).pool());
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
        return assure<Type>(id_generator::gen<Type>());
    }

private:
    std::vector<std::unique_ptr<pool_base_type>> pools_;
    entities_container entities_;

    template <typename Type>
    internal::storage_for_t<pool_base_type, Type>& assure(size_t idx) noexcept {
        if (idx >= pools_.size()) {
            pools_.resize(idx + 1);
        }
        if (pools_[idx] == nullptr) {
            pools_[idx] = std::make_unique<internal::storage_for_t<pool_base_type, Type>>();
        }
        return static_cast<internal::storage_for_t<pool_base_type, Type>&>(*pools_[idx]);
    }
};

}