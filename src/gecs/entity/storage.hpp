#pragma once

#include "sparse_set.hpp"

namespace gecs {

namespace internal {

template <typename StorageT>
class storage_iterator {
public:
    using container_type = typename StorageT::payload_container_type;
    using iterator_traits = std::iterator_traits<typename container_type::const_iterator>;
    using value_type = typename iterator_traits::value_type;
    using pointer = typename iterator_traits::pointer;
    using reference = typename iterator_traits::reference;
    using difference_type = typename iterator_traits::difference_type;
    using iterator_category = std::random_access_iterator_tag;

    constexpr storage_iterator(): container_(nullptr), offset_(0u) {}
    constexpr storage_iterator(const container_type* container): container_(container), offset_(0u) {}
    constexpr storage_iterator(const container_type* container, difference_type offset) : container_(container), offset_(offset) {}

    constexpr pointer operator->() const noexcept {
        return container_->data() + index();
    }

    constexpr reference operator*() const noexcept {
        return *operator->();
    }

    constexpr difference_type index() const noexcept {
        return offset_ - 1;
    }

    constexpr pointer data() const noexcept {
        return container_ ? container_->data() : nullptr;
    }

    constexpr storage_iterator& operator+=(const difference_type step) noexcept {
        return offset_ -= step, *this;
    }

    constexpr storage_iterator& operator-=(const difference_type step) noexcept {
        return operator+=(-step);
    }

    constexpr storage_iterator operator+(const difference_type step) const noexcept {
        storage_iterator copy = *this;
        return copy += step;
    }

    constexpr storage_iterator operator-(const difference_type step) const noexcept {
        return operator+(-step);
    }

    constexpr storage_iterator operator++(int) noexcept {
        storage_iterator copy = *this;
        return ++(*this), copy;
    }

    constexpr storage_iterator& operator++() noexcept {
        return --offset_, *this;
    }

    constexpr storage_iterator operator--(int) noexcept {
        storage_iterator copy = *this;
        return --(*this), copy;
    }

    constexpr storage_iterator& operator--() noexcept {
        return ++offset_, *this;
    }
    
    constexpr bool operator==(const storage_iterator& o) const noexcept {
        return o.container_ == container_ && o.offset_ == offset_;
    }

    constexpr bool operator!=(const storage_iterator& o) const noexcept {
        return !(*this == o);
    }

    constexpr reference operator[](const difference_type index) const noexcept {
        return data()[index() - index];
    }

private:
    const container_type* container_;
    difference_type offset_;
};

}

template <typename EntityT, typename Payload, size_t PageSize>
class basic_storage: public basic_sparse_set<EntityT, PageSize> {
public:
    using type = basic_storage<EntityT, Payload, PageSize>;
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

    auto begin() const {
        return internal::storage_iterator<type>{&packed_, static_cast<internal::storage_iterator<type>::difference_type>(packed_.size())};
    }

    auto end() const {
        return internal::storage_iterator<type>{&packed_, 0};
    }

    auto cend() const {
        return end();
    }

    auto cbegin() const {
        return begin();
    }

    auto rbegin() const {
        return std::make_reverse_iterator(end());
    }

    auto rend() const {
        return std::make_reverse_iterator(begin());
    }

    auto crbegin() const {
        return rbegin();
    }

    auto crend() const {
        return rend();
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
    using base_type = basic_sparse_set<EntityT, PageSize>;
    using size_type = typename base_type::size_type;

    void remove(EntityT entity) noexcept {
        if (!base_type::contain(entity)) {
            return;
        }

        auto& ref = base_type::pump(entity);
        ref = internal::entity_inc_version(ref);
        length_ --;
    }

    size_type size() const noexcept {
        return length_;
    }

    size_type capacity() const noexcept {
        return base_type::capacity();
    }

    size_type base_size() const noexcept {
        return base_type::size();
    }

    void reserve(size_type size) const noexcept {
        base_type::reserve(size);
    }

    //! @brief create a new entity or reuse old entity
    //! @return 
    EntityT emplace() noexcept {
        length_ ++;

        if (length_ <= base_type::size()) {
            return static_cast<EntityT>(base_type::data()[length_ - 1]);
        } else {
            return base_type::insert(static_cast<EntityT>(base_type::size()));
        }
    }

    bool empty() const noexcept {
        return length_ == 0;
    }

private:
    size_type length_ = 0;
};

}