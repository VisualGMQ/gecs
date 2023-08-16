#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "gecs/entity/querier.hpp"
#include "gecs/entity/world.hpp"

using namespace gecs;

struct Comp {
    int value;
};

TEST_CASE("querier") {
    basic_world<config::Entity, config::PageSize> world;

    SECTION("init state check") {
        auto querier = world.query<Comp>();
        REQUIRE(querier.begin() == querier.end());
        REQUIRE(querier.size() == 0);
    }

    for (int i = 0; i < 5; i++) {
        auto entity = world.create();
        world.emplace<Comp>(entity, Comp{i});
    }
    
    SECTION("non-mutable query") {
        auto querier = world.query<Comp>();
        REQUIRE(querier.size() == 5);
        auto it = querier.begin();
        for (int i = 4; i >= 0; i--) {
            auto& component = std::get<1>(*it);
            REQUIRE(component.value == i);
            REQUIRE(std::is_const_v<std::remove_reference_t<decltype(component)>>);
            it ++;
        }
        auto it_end = querier.end();
        REQUIRE(it == querier.end());
    }

    SECTION("mutable query") {
        auto querier = world.query<mut<Comp>>();
        REQUIRE(querier.size() == 5);
        auto it = querier.begin();
        for (int i = 4; i >= 0; i--) {
            auto& component = std::get<1>(*it);
            REQUIRE(component.value == i);
            REQUIRE_FALSE(std::is_const_v<std::remove_reference_t<decltype(component)>>);
            it ++;
        }
        auto it_end = querier.end();
        REQUIRE(it == querier.end());

    }
}

