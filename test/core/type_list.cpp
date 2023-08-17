#include "gecs/core/type_list.hpp"

#include <type_traits>

using namespace gecs;

using list1 = type_list<float, double, int>;

using t = type_list_element_t<0, list1>;

static_assert(std::is_same_v<type_list_element_t<0, list1>, float>);
static_assert(std::is_same_v<type_list_element_t<1, list1>, double>);
static_assert(std::is_same_v<type_list_element_t<2, list1>, int>);

int main() {
    return 0;
}