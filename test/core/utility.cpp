#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "gecs/core/utility.hpp"

#include <string>

double d = 445;

struct POD {
    int a;
    float b;
    double c;
    std::string str;

    static POD create() {
        return {1, 2, d, "hello"};
    }
};

TEST_CASE("extract pod members") {
    POD pod = POD::create();
    static_assert(gecs::is_braces_constructible_v<POD, int, float, double, std::string>);
    static_assert(!gecs::is_braces_constructible_v<POD, float, float, double, std::string>);
    static_assert(!gecs::is_braces_constructible_v<POD, double, std::string>);
    auto members = gecs::extract_pod_members(pod);

    REQUIRE(std::tuple_size_v<decltype(members)> == 4);
}