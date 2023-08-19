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

/**
 * @brief storage
 * 
 * storage aim to bind entity and component together, support quickly way to access entity and component
 * 
 * @tparam EntityT  the entity type
 * @tparam Payload  the payload type
 * @tparam PageSize  then page size for basic_sparse_set
 * @tparam Allocator then allocator to allocate Payload
 */
template <typename EntityT, typename Payload, size_t PageSize, typename Allocator>
class basic_storage: public basic_sparse_set<EntityT, PageSize> {
public:
    using type = basic_storage<EntityT, Payload, PageSize, Allocator>;
    using payload_type = Payload;
    using entity_type = EntityT;
    using entity_numeric_type = typename internal::entity_traits<EntityT>::entity_type;
    using allocator_type = Allocator;
    using alloc_traits = std::allocator_traits<allocator_type>;
    using payload_container_type = std::vector<typename alloc_traits::pointer, typename alloc_traits::template rebind_alloc<typename alloc_traits::pointer>>;
    using base_type = basic_sparse_set<EntityT, PageSize>;
    using size_type = typename base_type::size_type;

    //! @brief bind a payload to an entity
    //! @warning payload can't exists, otherwise will abort
    template <typename... Args>
    Payload& insert(entity_type entity, Args&&... args) noexcept {
        ECS_ASSERT("storage already has entity", !base_type::contain(entity));

        base_type::insert(entity);
        auto idx = base_type::index(entity);
        return *(new(assure(idx)[idx]) Payload{std::forward<Args>(args)...});
    }

    //! @brief emplace a payload and bind it to an entity
    //! @warning payload can't exists, otherwise will abort
    template <typename... Args>
    Payload& emplace(entity_type entity, Args&&... args) noexcept {
        ECS_ASSERT("storage already has entity", !base_type::contain(entity));

        base_type::insert(entity);
        auto idx = this->index(entity);
        return *(new(assure(idx)[idx]) Payload{std::forward<Args>(args)...});
    }

    size_type size() const noexcept {
        return base_type::size();
    }

    //! @brief remove the entity and it's payload
    void remove(entity_type entity) noexcept override {
        ECS_ASSERT("entity not exists", base_type::contain(entity));

        auto idx = this->index(entity);
        payloads_[idx]->~Payload();
        std::swap(payloads_[idx], payloads_[base_type::size() - 1]);
        base_type::remove(entity);
    }

    //! @brief replace the entity and it's payload
    //! @warning the entity must exists, otherwise will abort
    template <typename... Args>
    Payload& replace(entity_type entity, Args&&... args) noexcept {
        ECS_ASSERT("entity not exists", base_type::contain(entity));

        base_type::insert(entity);
        auto idx = this->index(entity);
        auto ptr = assure(idx)[idx];
        ptr->~Payload();
        return *(new(ptr) Payload{std::forward<Args>(args)...});
    }

    size_type capacity() const noexcept {
        return payloads_.capacity();
    }

    bool empty() const noexcept {
        return payloads_.empty();
    }

    //! @warning access the payload from entity
    const Payload& operator[](entity_type entity) const noexcept {
        return *payloads_[base_type::index(entity)];
    }

    //! @warning access the payload from entity
    Payload& operator[](entity_type entity) noexcept {
        return const_cast<Payload&>(std::as_const(*this).operator[](entity));
    }

    auto find(entity_type entity) const noexcept {
        if (base_type::contain(entity)) {
            return internal::storage_iterator<type>{&payloads_, static_cast<typename internal::storage_iterator<type>::difference_type>(base_type::index(entity)) + 1};
        } else {
            return end();
        }
    }

    auto begin() const noexcept {
        return internal::storage_iterator<type>{&payloads_, static_cast<typename internal::storage_iterator<type>::difference_type>(payloads_.size())};
    }

    auto end() const noexcept {
        return internal::storage_iterator<type>{&payloads_, 0};
    }

    auto cend() const noexcept {
        return end();
    }

    auto cbegin() const noexcept {
        return begin();
    }

    auto rbegin() const noexcept {
        return std::make_reverse_iterator(end());
    }

    auto rend() const noexcept {
        return std::make_reverse_iterator(begin());
    }

    auto crbegin() const noexcept {
        return rbegin();
    }

    auto crend() const noexcept {
        return rend();
    }

    //! @warning get all payloads
    const payload_container_type& payloads() const noexcept {
        return payloads_;
    }

    //! @warning get all payloads
    payload_container_type& payloads() noexcept {
        return const_cast<payload_container_type&>(std::as_const(*this).payloads());
    }

    //! @warning remove all payloads
    void release() noexcept {
        size_t i = 0;
        allocator_type allocator{get_allocator()};
        while (i < base_type::size()) {
            payloads_[i]->~Payload();
            allocator.deallocate(payloads_[i], 1);
            payloads_[i] = nullptr;
            i++;
        }
        while (i < payloads_.size()) {
            allocator.deallocate(payloads_[i], 1);
            payloads_[i] = nullptr;
            i++;
        }
        payloads_.clear();
        base_type::clear();
    }

    ~basic_storage() {
        release();
    }

private:
    payload_container_type payloads_;

    payload_container_type& assure(size_type size) noexcept {
        size_t oldSize = payloads_.size();
        if (size >= payloads_.size()) {
            payloads_.resize(size + 1);
        }
        for (size_t i = oldSize; i < payloads_.size(); i++) {
            allocator_type allocator{get_allocator()};
            payloads_[i] = alloc_traits::allocate(allocator, 1);
        }
        return payloads_;
    }

    allocator_type get_allocator() const noexcept {
        return payloads_.get_allocator();
    }
};

template <typename EntityT, size_t PageSize>
class basic_storage<EntityT, EntityT, PageSize, void>: protected basic_sparse_set<EntityT, PageSize> {
public:
    using entity_type = EntityT;
    using entity_numeric_type = typename internal::entity_traits<EntityT>::entity_type;
    using base_type = basic_sparse_set<EntityT, PageSize>;
    using size_type = typename base_type::size_type;

    void remove(entity_type entity) noexcept override {
        if (!base_type::contain(entity)) {
            return;
        }

        auto& ref = base_type::pump(entity);
        ref = internal::entity_inc_version(ref);
        length_ --;
    }

    bool contain(entity_type entity) const noexcept {
        return base_type::contain(entity);
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
    entity_type emplace() noexcept {
        length_ ++;

        if (length_ <= base_type::size()) {
            return static_cast<entity_type>(base_type::data()[length_ - 1]);
        } else {
            return base_type::insert(static_cast<entity_type>(base_type::size()));
        }
    }

    bool empty() const noexcept {
        return length_ == 0;
    }

private:
    size_type length_ = 0;
};

}