#pragma once

#include <cstdint>

namespace gecs {

namespace config {
    enum class Entity: uint32_t {};
    constexpr uint32_t PageSize = 4096;
}

}