#pragma once

#include "gecs/entity/entity.hpp"
#include "gecs/core/utility.hpp"

#include <array>
#include <cassert>
#include <limits>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

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

    constexpr sparse_set_iterator() : container_(nullptr), offset_(0u) {}

    constexpr sparse_set_iterator(const container_type* container)
        : container_(container), offset_(0u) {}

    constexpr sparse_set_iterator(const container_type* container,
                                  difference_type offset)
        : container_(container), offset_(offset) {}

    constexpr pointer operator->() const noexcept {
        return container_->data() + index();
    }

    constexpr reference operator*() const noexcept { return *operator->(); }

    constexpr difference_type index() const noexcept { return offset_ - 1; }

    constexpr pointer data() const noexcept {
        return container_ ? container_->data() : nullptr;
    }

    constexpr sparse_set_iterator& operator+=(
        const difference_type step) noexcept {
        return offset_ -= step, *this;
    }

    constexpr sparse_set_iterator& operator-=(
        const difference_type step) noexcept {
        return operator+=(-step);
    }

    constexpr sparse_set_iterator operator+(
        const difference_type step) const noexcept {
        sparse_set_iterator copy = *this;
        return copy += step;
    }

    constexpr sparse_set_iterator operator-(
        const difference_type step) const noexcept {
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

}  // namespace internal

/**
 * @brief sparse set for storing entity
 * @tparam EntityT  the entity type
 * @tparam PageSize  the page size
 **/
template <typename EntityT, size_t PageSize>
class basic_sparse_set {
public:
    using type = basic_sparse_set;
    using entity_numeric_type =
        typename internal::entity_traits<EntityT>::entity_type;
    using entity_type = EntityT;
    using packed_container_type = std::vector<entity_numeric_type>;
    using page_type = std::array<size_t, PageSize>;
    using sparse_container_type = std::vector<page_type>;
    using size_type = typename packed_container_type::size_type;
    static constexpr typename page_type::value_type null_sparse_data =
        std::numeric_limits<typename page_type::value_type>::max();
    using iterator =
        internal::sparse_set_iterator<basic_sparse_set<EntityT, PageSize>>;
    using const_iterator = iterator;

    //! @brief insert an entity
    entity_type insert(entity_type entity) noexcept {
        using traits = internal::entity_traits<entity_type>;
        GECS_ASSERT(traits::entity_mask != internal::entity_id(entity), "invalid entity id");

        auto id = internal::entity_id(entity);
        packed_.push_back(internal::entity_to_integral(entity));
        assure(page(id))[offset(id)] = packed_.size() - 1u;
        return internal::construct_entity<entity_type>(
            0, static_cast<typename traits::entity_mask_type>(packed_.size() -
                                                              1u));
    }

    //! @brief remove an entity
    virtual void remove(entity_type entity) noexcept {
        if (!contain(entity)) {
            return;
        }

        auto id = internal::entity_id(entity);
        auto& ref = sparse_ref(id);
        auto pos = ref;
        packed_[pos] = std::move(packed_.back());
        sparse_ref(internal::entity_id(packed_[pos])) = pos;
        ref = null_sparse_data;
        packed_.pop_back();
    }

    //! @brief pump a entity to the idx and return it
    entity_numeric_type& pump(entity_type src, entity_type dst) noexcept {
        GECS_ASSERT(!empty(), "sparse set must not empty when pump element");

        auto id = internal::entity_id(src);
        auto& ref1 = sparse_ref(id);
        auto& ref2 = sparse_ref(internal::entity_id(dst));
        std::swap(packed_[ref2], packed_[ref1]);
        std::swap(ref1, ref2);
        return packed_[ref1];
    }

    //! @brief get the entity index in packed
    size_t index(entity_type entity) const noexcept {
        auto id = internal::entity_id(entity);
        auto page = this->page(id);
        GECS_ASSERT(page < sparse_.size(), "entity must exists in sparse set when call index");
        return sparse_[page][offset(id)];
    }

    //! @brief does contain an entity
    bool contain(entity_type entity) const noexcept {
        auto id = internal::entity_id(entity);
        auto page = this->page(id);
        if (page >= sparse_.size()) {
            return false;
        } else {
            auto pos = sparse_[page][offset(id)];
            return pos != null_sparse_data && packed_[pos] != null_entity &&
                   packed_[pos] == entity;
        }
    }

    const_iterator begin() const noexcept {
        return internal::sparse_set_iterator<type>{
            &packed_,
            static_cast<
                typename internal::sparse_set_iterator<type>::difference_type>(
                packed_.size())};
    }

    const_iterator end() const noexcept {
        return internal::sparse_set_iterator<type>{&packed_, 0};
    }

    iterator begin() noexcept {
        return const_cast<iterator&&>(std::as_const(*this).begin());
    }

    iterator end() noexcept {
        return const_cast<iterator&&>(std::as_const(*this).end());
    }

    const_iterator cend() const noexcept { return end(); }

    const_iterator cbegin() const noexcept { return begin(); }

    auto rbegin() const noexcept { return std::make_reverse_iterator(end()); }

    auto rend() const noexcept { return std::make_reverse_iterator(begin()); }

    auto crbegin() const noexcept { return rbegin(); }

    auto crend() const noexcept { return rend(); }

    auto size() const noexcept { return packed_.size(); }

    auto capacity() const noexcept { return packed_.capacity(); }

    auto shrink_to_fit() noexcept {
        packed_.shrink_to_fit();
        sparse_.shrink_to_fit();
    }

    void clear() noexcept {
        packed_.clear();
        sparse_.clear();
    }

    bool empty() const noexcept { return packed_.empty(); }

    void reserve(size_type size) noexcept { packed_.reserve(size); }

    const entity_type& back() const noexcept { return packed_.back(); }

    auto data() const noexcept { return packed_.data(); }

    auto data() noexcept { return std::as_const(*this).data(); }

    entity_type& back() noexcept {
        return const_cast<entity_type&>(std::as_const(*this).back());
    }

    internal::sparse_set_iterator<basic_sparse_set> find(
        entity_type entity) noexcept {
        return {this, index(entity)};
    }

    const packed_container_type& packed() const noexcept { return packed_; }

    packed_container_type& packed() noexcept {
        return const_cast<packed_container_type&>(
            std::as_const(*this).packed());
    }

    virtual ~basic_sparse_set() = default;

private:
    packed_container_type packed_;
    sparse_container_type sparse_;

    size_t page(entity_numeric_type id) const noexcept { return id / PageSize; }

    size_t offset(entity_numeric_type id) const noexcept {
        return id % PageSize;
    }

    const size_t* sparse_ptr(entity_numeric_type id) const noexcept {
        auto page = this->page(id);
        return page < sparse_.size() ? &sparse_[page][offset(id)] : nullptr;
    }

    const size_t& sparse_ref(entity_numeric_type id) const noexcept {
        return sparse_[page(id)][offset(id)];
    }

    size_t* sparse_ptr(entity_numeric_type id) noexcept {
        return const_cast<size_t*>(std::as_const(*this).sparse_ptr(id));
    }

    size_t& sparse_ref(entity_numeric_type id) noexcept {
        return const_cast<size_t&>(std::as_const(*this).sparse_ref(id));
    }

    auto& assure(size_type page) noexcept {
        if (page >= sparse_.size()) {
            auto old_size = sparse_.size();
            sparse_.resize(page + 1u);
            for (size_type i = old_size; i < sparse_.size(); i++) {
                std::uninitialized_fill(std::begin(sparse_[i]),
                                        std::end(sparse_[i]), null_sparse_data);
            }
        }
        return sparse_[page];
    }
};

}  // namespace gecs