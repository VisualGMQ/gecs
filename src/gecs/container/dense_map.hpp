#pragma once

#include "compress_pair.hpp"
#include "core/utility.hpp"

#include <vector>
#include <type_traits>
#include <utility>

namespace gecs {

namespace internal {

template <typename Container>
class dense_map_iterator final {
public:
    using container_type = Container;
    using pointer = Container::const_pointer;
    using const_pointer = Container::const_pointer;
    using reference = typename container_type::const_reference;
    using const_reference = typename container_type::const_reference;
    using difference_type = typename container_type::difference_type;
    using value_type = typename container_type::value_type;
    using iterator_category = std::random_access_iterator_tag;

    constexpr dense_map_iterator(const Container& container, difference_type offset) noexcept : container_(&container), offset_(offset) {}
    constexpr dense_map_iterator(const Container& container, container_type::const_iterator it) noexcept : container_(&container), offset_(std::distance(container.begin(), it)) {}

    constexpr reference operator*() noexcept {
        return *(container_->begin() + index());
    }

    constexpr reference operator*() const noexcept {
        return std::as_const(*this).operator*();
    }

    constexpr const pointer operator->() const noexcept {
        return &operator*();
    }

    constexpr pointer operator->() noexcept {
        return const_cast<pointer>(std::as_const(*this).operator->());
    }

    constexpr const_pointer data() const noexcept {
        return container_ ? container_->data() : nullptr;
    }

    constexpr pointer data() noexcept {
        return const_cast<pointer>(std::as_const(*this).data());
    }

    constexpr auto& operator+=(const difference_type step) noexcept {
        return offset_ -= step, *this;
    }

    constexpr auto& operator-=(const difference_type step) noexcept {
        return operator+=(-step);
    }

    constexpr auto operator+(const difference_type step) const noexcept {
        auto copy = *this;
        return copy += step;
    }

    constexpr auto operator-(const difference_type step) const noexcept {
        return operator+(-step);
    }

    constexpr auto operator++(int) noexcept {
        auto copy = *this;
        return ++(*this), copy;
    }

    constexpr auto& operator++() noexcept {
        return --offset_, *this;
    }

    constexpr auto operator--(int) noexcept {
        sparse_set_iterator copy = *this;
        return --(*this), copy;
    }

    constexpr auto& operator--() noexcept {
        return ++offset_, *this;
    }
    
    constexpr bool operator==(const dense_map_iterator& o) const noexcept {
        return o.container_ == container_ && o.offset_ == offset_;
    }

    constexpr bool operator!=(const dense_map_iterator& o) const noexcept {
        return !(*this == o);
    }

    constexpr reference operator[](const difference_type index) const noexcept {
        return data()[index() - index];
    }

private:
    const Container* container_;
    Container::difference_type offset_;

    inline size_t index() const {
        return offset_ - 1;
    }
};

}

template <typename Key, typename Value, typename Hasher>
class dense_map final {
public:
    using element_type = std::pair<Key, Value>
    using sparse_container_type = std::vector<size_t>;
    using packed_container_type = std::vector<element_type>;
    using iterator = internal::dense_map_iterator<dense_map<Key, Value, Hasher>>;
    using const_iterator = iterator;

    dense_map(size_t init_hash_size = 8) {
        sparse_.resize(init_hash_size, null_sparse_element);
    }

    template <typename T, typename U>
    void insert_or_donothing(T&& key, U&& value) {
        size_t index = this->index(key);
        if (!is_null(sparse_[index])) {
            return;
        } else {
            packed_.First().emplace_back(std::make_pair<Key, Value>(std::forward<Key>(key), std::forward<Value>(value)));
            sparse_[index] = packed_.size() - 1;
        }
    }

    template <typename T, typename U>
    void insert_or_replace(T&& key, U&& value) {
        size_t index = this->index(key);
        packed_.First().emplace_back(std::make_pair<Key, Value>(std::forward<Key>(key), std::forward<Value>(value)));
        sparse_[index] = packed_.size() - 1;
    }

    iterator begin() {
        return iterator{&packed_, packed_.size()};
    }

    cosnt_iterator begin() const {
        return std::as_const(*this).begin();
    }

    iterator end() {
        return iterator{&packed_, 0};
    }

    const_iterator end() const {
        return std::as_const(*this).end();
    }

    const_iterator cend() const {
        return end();
    }

    const_iterator cbegin() const {
        return begin();
    }

    const_iterator find(const Key& key) const {
        size_t index = this->index(key);
        if (is_null(sparse_[index])) {
            return cend();
        } else {
            return const_iterator{&packed_, index + 1};
        }
    }

    iterator find(const Key& key) {
        return const_cast<iterator&&>(std::as_const(*this).find(key));
    }

    void remove(const Key& key) {
        size_t index = sparse_[this->index(key)];
        index = null_sparse_element;
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

private:
    CompressPair<packed_container_type, Hasher> packed_;
    sparse_container_type sparse_;
    float threshold_ = 0.85;

    static constexpr auto null_sparse_element = std::numeric_limits<sparse_container_type::value_type>::max();

    inline bool is_null(size_t elem) {
        return elem == null_sparse_element;
    }

    inline size_t index(const Key& key) {
        return quick_mod<size_t>(Hasher{}(key), sparse_.size());
    }
};

}