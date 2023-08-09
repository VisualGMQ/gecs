#include "gecs/signal/delegate.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace gecs;

double Multiply2(float value) {
    return value * 2.0;
}

bool OrderedParams(std::string, float, double) {
    return true;
}

struct Person final {
    const std::string& GetName() const { return name; }

    std::string name;
    float height;
};

TEST_CASE("delegate") {
    SECTION("simple global function delegate") {
        Delegate<double(float)> d;
        d.Connect<&Multiply2>();
        REQUIRE(d(2.0) == 4.0);
    }

    SECTION("delegate with payload(corialization)") {
        Delegate<double()> d;
        float pre_value = 123.0;
        d.Connect<&Multiply2>(pre_value);
        REQUIRE(d() == 246.0);
    }

    SECTION("delegate with ordered arguments") {
        Delegate<bool(float, std::string, double)> d;
        d.Connect<&OrderedParams>(std::index_sequence<1, 0, 2>{});
        REQUIRE(d(1, "", 3.0));
    }

    SECTION("work with class") {
        Person person;
        person.name = "foo";
        person.height = 122;

        Delegate<std::string(void)> d1;
        d1.Connect<&Person::name>(person);
        REQUIRE(d1() == "foo");

        Delegate<std::string(Person&)> d2;
        d2.Connect<&Person::name>();
        REQUIRE(d2(person) == "foo");

        Delegate<const std::string&(Person&)> d3;
        d3.Connect<&Person::GetName>();
        REQUIRE(d3(person) == "foo");
    }
}