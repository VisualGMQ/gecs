#pragma once

#include "entity.hpp"
#include "querier.hpp"
#include "gecs/core/singlton.hpp"
#include "gecs/core/utility.hpp"

#include <type_traits>
#include <memory>

namespace gecs {

namespace internal {

template <typename T>
struct default_construct_loader {
    T operator()() const {
        return T{};
    }
};

template <typename T>
class resource_cache final : public singlton<resource_cache<T>, false, internal::default_construct_loader<resource_cache<T>>> {
public:
    using type = std::decay_t<T>;
    using type_reference = type&;
    using type_const_reference = const type&;

    bool has() const noexcept {
        return data_;
    }

    template <typename... Args>
    type_reference& emplace(Args&&... args) noexcept {
        data_ = std::make_unique<type>(std::forward<Args>(args)...);
        return *data_;
    }

    void remove() noexcept {
        data_.reset();
    }

    type_const_reference get() const noexcept {
        ECS_ASSERT("resource not exists", data_);
        return *data_;
    }

    type_reference get() noexcept {
        ECS_ASSERT("resource not exists", data_);
        return const_cast<type_reference>(std::as_const(*this).get());
    }

private:
    std::unique_ptr<type> data_;
};

}

template <typename T>
class resource final {
public:
    using resource_type = internal::remove_mut_t<T>;
    using resource_reference_t = resource_type&;
    using resource_const_reference_t = const resource_type&;

    std::conditional_t<internal::is_mutable_v<T>, resource_reference_t, resource_const_reference_t> get() {
       return internal::resource_cache<internal::remove_mut_t<T>>::instance().get();
    }

    auto& operator*() noexcept {
        return get();
    }

    auto operator->() noexcept {
        return &operator*();
    }
};

}