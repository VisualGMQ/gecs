#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "gecs/entity/querier.hpp"
#include "gecs/entity/world.hpp"

using namespace gecs;

struct Comp1 {
    int value;
};

struct Comp2 {
    std::string name;
};

struct Comp3 {
    float value;
};

using world = basic_world<config::Entity, config::PageSize>;

TEST_CASE("querier misc") {
    world world;

    SECTION("init state check") {
        auto querier = world.query<Comp1>();
        REQUIRE(querier.begin() == querier.end());
        REQUIRE(querier.size() == 0);
    }
}

TEST_CASE("single querier") {
    world world;

    for (int i = 0; i < 5; i++) {
        auto entity = world.create();
        world.emplace<Comp1>(entity, Comp1{i});
    }
    
    SECTION("non-mutable single query") {
        auto querier = world.query<Comp1>();
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

    SECTION("mutable single query") {
        auto querier = world.query<mut<Comp1>>();
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

TEST_CASE("multiple querier") {
    world world;

    std::array<world::entity_type, 5> entities = {
        world.create(),
        world.create(),
        world.create(),
        world.create(),
        world.create(),
    };

    world.emplace<Comp1>(entities[0], Comp1{8});
    world.emplace<Comp1>(entities[1], Comp1{2});
    world.emplace<Comp1>(entities[2], Comp1{0});
    world.emplace<Comp1>(entities[3], Comp1{9});
    world.emplace<Comp2>(entities[1], Comp2{"ent2"});
    world.emplace<Comp2>(entities[4], Comp2{"ent3"});

    SECTION("non-mutable query") {
        auto querier = world.query<Comp1, Comp2>();
        auto begin = querier.begin();

        REQUIRE(querier.size() == 1);
        REQUIRE(std::get<1>(*begin).value == 2);
        REQUIRE(std::get<2>(*begin).name == "ent2");
        REQUIRE(begin + 1 == querier.end());
    }

    SECTION("invalid component query") {
        auto querier = world.query<Comp3>();

        REQUIRE(querier.size() == 0);
        REQUIRE(querier.begin() == querier.end());
    }

    SECTION("sort") {
        auto querier = world.query<Comp1>().sort_by<Comp1>([](const Comp1& c1, const Comp1& c2){
            return c1.value < c2.value;
        });
        auto it = querier.begin();
        REQUIRE(std::get<1>(*it).value == 0);
        it ++;
        REQUIRE(std::get<1>(*it).value == 2);
        it ++;
        REQUIRE(std::get<1>(*it).value == 9);
        it ++;
        REQUIRE(std::get<1>(*it).value == 8);
    }
}
