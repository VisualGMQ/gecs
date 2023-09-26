#pragma once

#include "gecs/entity/entity.hpp"

#include <cstddef>
#include <cstdint>

#ifndef ENTITY_NUMERIC_TYPE
#define ENTITY_NUMERIC_TYPE uint32_t
#endif

#ifndef SPARSE_PAGE_SIZE
#define SPARSE_PAGE_SIZE 4096
#endif

#ifndef GECS_GET_TYPE_INFO
#define GECS_GET_TYPE_INFO(type) nullptr
#endif

namespace gecs::internal {

struct fake_reference_any final {
    template <typename T>
    fake_reference_any(const T&) {}

    fake_reference_any() = default;
};

}

#ifndef GECS_REFERENCE_ANY
#define GECS_REFERENCE_ANY ::gecs::internal::fake_reference_any
#endif

namespace gecs {

namespace config {

enum class Entity : ENTITY_NUMERIC_TYPE {};
constexpr uint32_t PageSize = SPARSE_PAGE_SIZE;

inline bool operator==(Entity e1, Entity e2) {
    return internal::entity_to_integral(e1) == internal::entity_to_integral(e2);
}

inline bool operator!=(Entity e1, Entity e2) {
    return !(e1 == e2);
}

inline bool operator==(Entity e1, uint64_t num) {
    return internal::entity_to_integral(e1) == num;
}

inline bool operator!=(Entity e1, uint64_t num) {
    return !(e1 == num);
}

inline bool operator==(uint64_t num, Entity e) {
    return e == num;
}

inline bool operator!=(uint64_t num, Entity e) {
    return !(e == num);
}

using id_type = size_t;



}  // namespace config

}  // namespace gecs
