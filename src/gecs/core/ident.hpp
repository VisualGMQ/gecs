#pragma once

#include "gecs/config/config.hpp"

namespace gecs {

struct id_generator final {
public:
    using value_type = config::id_type;

    template <typename Type>
    static value_type gen() noexcept {
        static value_type value = curr_ ++;
        return value;
    }

private:
    inline static value_type curr_ = {};
};

}