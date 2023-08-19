#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "gecs/signal/sigh.hpp"

using namespace gecs;

int gCallCount = 0;

TEST_CASE("sigh") {
    sigh<void(void)> sigh;

    {
        auto constexpr f1 = +[](){
            gCallCount += 1;
        };
        delegate<void()> d;
        d.connect<f1>();
        sigh += d;
    }

    {
        auto constexpr f = +[](){
            gCallCount += 2;
        };
        delegate<void()> d;
        d.connect<f>();
        sigh += d;
    }

    {
        auto constexpr f = +[](){
            gCallCount += 3;
        };
        delegate<void()> d;
        d.connect<f>();
        sigh += d;
    }

    sigh.trigger();

    REQUIRE(sigh.size() == 3);
    REQUIRE(!sigh.empty());
    REQUIRE(gCallCount == 6);
}