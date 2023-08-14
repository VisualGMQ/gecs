#pragma once

#include "config/config.hpp"

namespace gecs {

struct id_generator final {
public:
    using value_type = config::id_type;

    template <typename Type>
    value_type gen() noexcept {
        static value_type value = curr_ ++;
        return value;
    }

private:
    static value_type curr_ = {};
};

}