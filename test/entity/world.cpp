#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "gecs/entity/fwd.hpp"

using namespace gecs;

struct Comp1 final {
    float value;
};

struct Comp2 final {
    std::string value;
};

TEST_CASE("world") {
    world w;

    SECTION("misc") {
        auto entity1 = w.create();

        REQUIRE(entity1 == 0);
        REQUIRE_FALSE(w.contain<Comp1>(entity1));
        REQUIRE_FALSE(w.contain<Comp2>(entity1));
        REQUIRE(w.alive(entity1));

        auto entity2 = w.create();
        REQUIRE(entity2 == 1);
        REQUIRE(w.alive(entity2));

        auto& comp1 = w.emplace<Comp1>(entity1, Comp1{123.0f});
        REQUIRE(comp1.value == 123.0f);
        REQUIRE(w.contain<Comp1>(entity1));

        auto& comp2 = w.replace<Comp1>(entity1, Comp1{456.0f});
        REQUIRE(comp2.value == 456.0f);
        REQUIRE(w.contain<Comp1>(entity1));

        REQUIRE_FALSE(w.contain<Comp1>(entity2));
        REQUIRE_FALSE(w.contain<Comp2>(entity2));

        auto entity3 = w.create();
        w.emplace<Comp1>(entity3, Comp1{999.0f});
        w.emplace<Comp2>(entity3, Comp2{"ent3"});

        REQUIRE(w.contain<Comp1>(entity3));
        REQUIRE(w.contain<Comp2>(entity3));

        REQUIRE(w.get<Comp1>(entity3).value == 999.0f);
        REQUIRE(w.get<Comp2>(entity3).value == "ent3");

        w.remove<Comp1>(entity3);
        REQUIRE_FALSE(w.contain<Comp1>(entity3));
        REQUIRE(w.contain<Comp2>(entity3));

        REQUIRE(w.size() == 3);

        w.destroy(entity3);
        REQUIRE_FALSE(w.alive(entity3));
        REQUIRE(w.alive(entity1));
        REQUIRE(w.alive(entity2));
        REQUIRE(w.size() == 2);
    }

    SECTION("reuse") {
        auto entity = w.create();
        auto entity2 = w.create();
        auto entity3 = w.create();
        w.emplace<Comp1>(entity, Comp1{1});
        w.emplace<Comp1>(entity2, Comp1{2});
        w.emplace<Comp1>(entity3, Comp1{3});

        REQUIRE(std::get<1>(*w.query<Comp1>().begin()).value == 3);
        REQUIRE(std::get<1>(*(w.query<Comp1>().begin() + 1)).value == 2);
        REQUIRE(std::get<1>(*(w.query<Comp1>().begin() + 2)).value == 1);

        w.destroy(entity);
        entity = w.create();
        w.emplace<Comp1>(entity, Comp1{4});
        REQUIRE(std::get<1>(*w.query<Comp1>().begin()).value == 4);
        REQUIRE(std::get<1>(*(w.query<Comp1>().begin() + 1)).value == 2);
        REQUIRE(std::get<1>(*(w.query<Comp1>().begin() + 2)).value == 3);
    }

    SECTION("test") {
        auto entity0 = w.create(); // 0
        auto entity1 = w.create(); // 1
        auto entity2 = w.create(); // 2
        auto entity3 = w.create(); // 3
        auto entity4 = w.create(); // 4
        auto entity5 = w.create(); // 5
        auto entity6 = w.create(); // 6

        w.destroy(entity2);
        auto entity1048578 = w.create();
        w.destroy(entity3);
        auto entity1048579 = w.create();
        w.destroy(entity4);
        w.destroy(entity5);
        auto entity1048580 = w.create();
        w.destroy(entity6);
        auto lastEntity = w.create();
    }
}
