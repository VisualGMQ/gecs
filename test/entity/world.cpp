#pragma once

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
