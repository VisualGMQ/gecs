#pragma once

#include "gecs/entity/entity.hpp"

#include <cstdint>

namespace gecs {

namespace config {

enum class Entity: uint32_t {};
constexpr uint32_t PageSize = 4096;

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


}

}