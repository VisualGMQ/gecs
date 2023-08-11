#pragma once

#include "storage.hpp"

namespace gecs {

namespace internal {

}

template <typename EntityT, size_t PageSize>
class World final {
public:
    World() = default;

private:
    basic_storage<EntityT, EntityT> entities_;
};

}