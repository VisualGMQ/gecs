#include "gecs/core/type_list.hpp"

#include <type_traits>

using namespace gecs;

using list1 = type_list<float, double, int>;

using t = list_element_t<list1, 0>;

static_assert(std::is_same_v<list_element_t<list1, 0>, float>);
static_assert(std::is_same_v<list_element_t<list1, 1>, double>);
static_assert(std::is_same_v<list_element_t<list1, 2>, int>);

int main() {
    return 0;
}