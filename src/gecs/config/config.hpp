#pragma once

#include "gecs/entity/entity.hpp"

#include <cstdint>
#include <cstddef>


#ifndef ENTITY_NUMERIC_TYPE
#define ENTITY_NUMERIC_TYPE uint32_t
#endif

#ifndef SPARSE_PAGE_SIZE
#define SPARSE_PAGE_SIZE 4096
#endif

namespace gecs {

namespace config {

enum class Entity: ENTITY_NUMERIC_TYPE {};
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

}

}
