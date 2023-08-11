#pragma once

#include "entity.hpp"

#include <tuple>
#include <array>
#include <vector>
#include <type_traits>
#include <cassert>
#include <memory>

namespace gecs {

namespace internal {

template <typename SparseSetT>
class sparse_set_iterator final {
public:
    using container_type = typename SparseSetT::packed_container_type;
    using pointer = typename container_type::const_pointer;
    using reference = typename container_type::const_reference;
    using difference_type = typename container_type::difference_type;
    using value_type = typename container_type::value_type;
    using iterator_category = std::random_access_iterator_tag;

    constexpr sparse_set_iterator(): container_(nullptr), offset_(0u) {}
    constexpr sparse_set_iterator(const container_type* container): container_(container), offset_(0u) {}
    constexpr sparse_set_iterator(const container_type* container, difference_type offset) : container_(container), offset_(offset) {}

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

    constexpr sparse_set_iterator& operator+=(const difference_type step) noexcept {
        return offset_ -= step, *this;
    }

    constexpr sparse_set_iterator& operator-=(const difference_type step) noexcept {
        return operator+=(-step);
    }

    constexpr sparse_set_iterator operator+(const difference_type step) const noexcept {
        sparse_set_iterator copy = *this;
        return copy += step;
    }

    constexpr sparse_set_iterator operator-(const difference_type step) const noexcept {
        return operator+(-step);
    }

    constexpr sparse_set_iterator operator++(int) noexcept {
        sparse_set_iterator copy = *this;
        return ++(*this), copy;
    }

    constexpr sparse_set_iterator& operator++() noexcept {
        return --offset_, *this;
    }

    constexpr sparse_set_iterator operator--(int) noexcept {
        sparse_set_iterator copy = *this;
        return --(*this), copy;
    }

    constexpr sparse_set_iterator& operator--() noexcept {
        return ++offset_, *this;
    }

    constexpr bool operator==(const sparse_set_iterator& o) const noexcept {
        return o.container_ == container_ && o.offset_ == offset_;
    }

    constexpr bool operator!=(const sparse_set_iterator& o) const noexcept {
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

template <typename EntityT, size_t PageSize>
class basic_sparse_set {
public:
    using type = basic_sparse_set;
    using entity_type = typename internal::entity_traits<EntityT>::entity_type;

    using packed_container_type = std::vector<entity_type>;
    using page_type = std::array<size_t, PageSize>;
    using sparse_container_type = std::vector<page_type>;
    using size_type = typename packed_container_type::size_type;
    static constexpr typename page_type::value_type null_sparse_data = std::numeric_limits<typename page_type::value_type>::max();
    using iterator = internal::sparse_set_iterator<basic_sparse_set<EntityT, PageSize>>;
    using const_iterator = iterator;

    EntityT insert(EntityT entity) {
        using traits = internal::entity_traits<EntityT>;
        ECS_ASSERT(traits::entity_mask != internal::entity_id(entity));

        auto id = internal::entity_id(entity);
        packed_.push_back(internal::entity_to_integral(entity));
        assure(page(id))[offset(id)] = packed_.size() - 1u;
        return internal::construct_entity<EntityT>(0, static_cast<traits::entity_mask_type>(packed_.size() - 1u));
    }

    void remove(EntityT entity) {
        if (!contain(entity)) {
            return;
        }

        auto id = internal::entity_id(entity);
        auto& ref = sparse_ref(id);
        auto pos = ref;
        ref = null_sparse_data;
        packed_[pos] = std::move(packed_.back());
        packed_.pop_back();
        sparse_ref(internal::entity_id(packed_[pos])) = pos;
    }

    void remove_back() {
        if (empty()) {
            return;
        }

        auto id = internal::entitiy_id(packed_.back());
        sparse_ref(id) = null_sparse_data;
        packed_.pop_back();
    }

    //! @brief pump a entity to end and return it
    entity_type& pump(EntityT entity) {
        ECS_ASSERT(!empty());

        auto id = internal::entity_id(entity);
        auto& ref1 = sparse_ref(id);
        auto& ref2 = sparse_ref(internal::entity_id(packed_.back()));
        std::swap(packed_.back(), packed_[ref1]);
        std::swap(ref1, ref2);
        return packed_.back();
    }

    size_t index(EntityT entity) const {
        auto id = internal::entity_id(entity);
        auto page = this->page(id);
        ECS_ASSERT(page < sparse_.size());
        return sparse_[page][offset(id)];
    }

    bool contain(EntityT entity) const {
        auto id = internal::entity_id(entity);
        auto page = this->page(id);
        if (page >= sparse_.size()) {
            return false;
        } else {
            auto pos = sparse_[page][offset(id)];
            return pos != null_sparse_data && packed_[pos] != internal::null_entity;
        }
    }

    const_iterator begin() const {
        return internal::sparse_set_iterator<type>{&packed_, static_cast<internal::sparse_set_iterator<type>::difference_type>(packed_.size())};
    }

    const_iterator end() const {
        return internal::sparse_set_iterator<type>{&packed_, 0};
    }

    iterator begin() {
        return const_cast<iterator&&>(std::as_const(*this).begin());
    }

    iterator end() {
        return const_cast<iterator&&>(std::as_const(*this).end());
    }

    const_iterator cend() const {
        return end();
    }

    const_iterator cbegin() const {
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

    auto size() const {
        return packed_.size();
    }

    auto capacity() const {
        return packed_.capacity();
    }

    auto shrink_to_fit() {
        packed_.shrink_to_fit();
        sparse_.shrink_to_fit();
    }

    void clear() {
        packed_.clear();
        sparse_.clear();
    }

    bool empty() const {
        return packed_.empty();
    }

    void reserve(size_type size) {
        packed_.reserve(size);
    }

    const EntityT& back() const {
        return packed_.back();
    }

    auto data() const {
        return packed_.data();
    }

    auto data() {
        return std::as_const(*this).data();
    }

    EntityT& back() {
        return const_cast<Entity&>(std::as_const(*this).back());
    }

    internal::sparse_set_iterator<basic_sparse_set> find(EntityT entity) {
        return {this, index(entity)};
    }

    virtual ~basic_sparse_set() = default;

private:
    packed_container_type packed_;
    sparse_container_type sparse_;

    size_t page(entity_type id) const {
        return id / PageSize;
    }

    size_t offset(entity_type id) const {
        return id % PageSize;
    }
    
    const size_t* sparse_ptr(entity_type id) const {
        auto page = this->page(id);
        return page < sparse_.size() ? &sparse_[page][offset(id)] : nullptr;
    }

    const size_t& sparse_ref(entity_type id) const {
        return sparse_[page(id)][offset(id)];
    }

    size_t* sparse_ptr(entity_type id) {
        return const_cast<size_t*>(std::as_const(*this).sparse_ptr(id));
    }

    size_t& sparse_ref(entity_type id) {
        return const_cast<size_t&>(std::as_const(*this).sparse_ref(id));
    }

    auto& assure(size_type page) {
        if (page >= sparse_.size()) {
            auto old_size = sparse_.size();
            sparse_.resize(page + 1u);
            for (size_type i = old_size; i < sparse_.size(); i++) {
                std::uninitialized_fill(std::begin(sparse_[i]), std::end(sparse_[i]), null_sparse_data);
            }
        }
        return sparse_[page];
    }
};

}