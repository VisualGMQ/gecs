#pragma once

#include "storage.hpp"
#include "core/ident.hpp"

namespace gecs {

namespace internal {

template <typename SparseSetT, typename Type>
struct storage_for;

template <typename EntityT, size_t PageSize, typename Type>
struct storage_for<basic_sparse_set<EntityT, PageSize>, Type> {
    using type = basic_storage<EntityT, Type, PageSize>;
};

template <typename SparseSetT, typename Type>
using storage_for_t = typename storage_for<SparseSetT, Type>::type;

}

template <typename EntityT, size_t PageSize>
class world final {
public:
    using entities_container = basic_storage<EntityT, EntityT>;
    using pool_base_type = basic_sparse_set<EntityT, PageSize>;

    EntityT create() noexcept {
        return entities_.create();
    }

    template <typename Type, typename... Args>
    Payload& emplace(EntityT entity, Args&&... args) noexcept {
        auto id = id_generator::gen<Type>();
        return assure(id).emplace(entity, std::forward<Args>(args)...);
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

private:
    std::vector<size_t, std::unique_ptr<pool_base_type>> pools_;
    entities_container entities_;

    template <typename Type>
    internal::storage_for_t<pool_base_type, Type>& assure(size_t idx) noexcept {
        if (idx >= pools_.size()) {
            pools_.resize(idx + 1);
        }
        return pools_[idx];
    }
};

}