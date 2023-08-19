#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "gecs/signal/sink.hpp"

using namespace gecs;

int gCallCount = 0;

template <int I>
void inc() {
    gCallCount += I;
}

TEST_CASE("sink with simple function") {
    sigh<void(void)> sigh;
    sink sink(sigh);

    sink.add<inc<1>>();
    sink.add<inc<2>>();
    sink.add<inc<3>>();

    SECTION("add") {
        REQUIRE(sigh.size() == 3);

        sigh.trigger();
        REQUIRE(gCallCount == 6);

        sink.clear();
        REQUIRE(sigh.empty());
    }

    SECTION("remove") {
        gCallCount = 0;

        sink.remove<inc<1>>();
        REQUIRE(sigh.size() == 2);
        sigh.trigger();
        REQUIRE(gCallCount == 5);

        sink.remove<inc<2>>();
        REQUIRE(sigh.size() == 1);
        sigh.trigger();
        REQUIRE(gCallCount == 8);
    }
}
