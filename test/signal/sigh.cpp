#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "gecs/signal/sigh.hpp"

using namespace gecs;

int gCallCount = 0;

void f1() {
    gCallCount += 1;
}

void f2() {
    gCallCount += 2;
}

void f3() {
    gCallCount += 3;
}

TEST_CASE("sigh") {
    sigh<void(void)> sigh;

    {
        delegate<void()> d;
        d.connect<f1>();
        sigh += d;
    }

    {
        delegate<void()> d;
        d.connect<f2>();
        sigh += d;
    }

    {
        delegate<void()> d;
        d.connect<f3>();
        sigh += d;
    }

    sigh.trigger();

    REQUIRE(sigh.size() == 3);
    REQUIRE(!sigh.empty());
    REQUIRE(gCallCount == 6);
}