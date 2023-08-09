#pragma once

#include "sparse_set.hpp"

namespace gecs {

template <typename EntityT, typename Payload, size_t PageSize>
class basic_storage: public basic_sparse_set<EntityT, PageSize> {
public:
    using payload_container_type = std::vector<Payload>;
    using base_type = basic_sparse_set<EntityT, PageSize>;
    using size_type = typename base_type::size_type;

    void insert(EntityT entity, const Payload& payload) noexcept {
        base_type::insert(entity);
        auto id = internal::entity_id(entity);
        assure(id)[id] = payload;
    }

    void insert(EntityT entity, Payload&& payload) noexcept {
        base_type::insert(entity);
        auto id = internal::entity_id(entity);
        assure(id)[id] = std::move(payload);
    }

    template <typename... Args>
    void emplace(EntityT entity, Args&&... args) noexcept {
        base_type::insert(entity);
        auto id = internal::entity_id(entity);
        assure(id)[id] = Payload{std::forward<Args>(args)...};
    }

    size_type size() const noexcept {
        return payloads_.size();
    }

    void remove(EntityT entity) {
        base_type::remove(entity);
    }

    size_type capacity() const noexcept {
        return payloads_.capacity();
    }

    bool empty() const noexcept {
        return payloads_.empty();
    }

    const Payload& operator[](EntityT entity) const noexcept {
        return payloads_[base_type::index(entity)];
    }

    Payload& operator[](EntityT entity) noexcept {
        return const_cast<Payload&>(std::as_const(*this).operator[](entity));
    }

private:
    payload_container_type payloads_;

    payload_container_type& assure(size_type size) {
        if (size >= payloads_) {
            payloads_.resize(size + 1);
        }
        return payloads_;
    }
};

template <typename EntityT, size_t PageSize>
class basic_storage<EntityT, EntityT, PageSize>: protected basic_sparse_set<EntityT, PageSize> {
public:
    using payload_container_type = std::vector<Payload>;
    using base_type = basic_sparse_set<EntityT, PageSize>;
    using size_type = typename base_type::size_type;

    void remove(EntityT entity) {
        if (!base_type::contain(entity)) {
            return;
        }

        auto& ref = base_type::pump(entity);
        ref = internal::entity_inc_version(ref);
        length_ --;
    }

    size_type size() const {
        return length_;
    }

    //! @brief create a new entity or reuse old entity
    //! @return 
    EntityT emplace() {
        length_ ++;

        if (length_ < base_type::size()) {
            return base_type::data()[length_ - 1];
        } else {
            return base_type::insert(base_type::size());
        }
    }

private:
    size_type length_ = 0;
};

}