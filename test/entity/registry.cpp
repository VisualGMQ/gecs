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

using trivial_owner = int;
using registry_type = gecs::basic_registry<trivial_owner, gecs::config::Entity, gecs::config::PageSize>;

trivial_owner owner;

TEST_CASE("registry") {
    registry_type reg(owner);

    SECTION("misc") {
        auto entity1 = reg.create();

        REQUIRE(entity1 == 0);
        REQUIRE_FALSE(reg.contain<Comp1>(entity1));
        REQUIRE_FALSE(reg.contain<Comp2>(entity1));
        REQUIRE(reg.alive(entity1));

        auto entity2 = reg.create();
        REQUIRE(entity2 == 1);
        REQUIRE(reg.alive(entity2));

        auto& comp1 = reg.emplace<Comp1>(entity1, Comp1{123.0f});
        REQUIRE(comp1.value == 123.0f);
        REQUIRE(reg.contain<Comp1>(entity1));

        auto& comp2 = reg.replace<Comp1>(entity1, Comp1{456.0f});
        REQUIRE(comp2.value == 456.0f);
        REQUIRE(reg.contain<Comp1>(entity1));

        REQUIRE_FALSE(reg.contain<Comp1>(entity2));
        REQUIRE_FALSE(reg.contain<Comp2>(entity2));

        auto entity3 = reg.create();
        reg.emplace<Comp1>(entity3, Comp1{999.0f});
        reg.emplace<Comp2>(entity3, Comp2{"ent3"});

        REQUIRE(reg.contain<Comp1>(entity3));
        REQUIRE(reg.contain<Comp2>(entity3));

        REQUIRE(reg.get<Comp1>(entity3).value == 999.0f);
        REQUIRE(reg.get<Comp2>(entity3).value == "ent3");

        reg.remove<Comp1>(entity3);
        REQUIRE_FALSE(reg.contain<Comp1>(entity3));
        REQUIRE(reg.contain<Comp2>(entity3));

        REQUIRE(reg.size() == 3);

        reg.destroy(entity3);
        REQUIRE_FALSE(reg.alive(entity3));
        REQUIRE(reg.alive(entity1));
        REQUIRE(reg.alive(entity2));
        REQUIRE(reg.size() == 2);
    }

    SECTION("reuse") {
        auto entity = reg.create();
        auto entity2 = reg.create();
        auto entity3 = reg.create();
        reg.emplace<Comp1>(entity, Comp1{1});
        reg.emplace<Comp1>(entity2, Comp1{2});
        reg.emplace<Comp1>(entity3, Comp1{3});

        REQUIRE(std::get<1>(*reg.query<Comp1>().begin()).value == 3);
        REQUIRE(std::get<1>(*(reg.query<Comp1>().begin() + 1)).value == 2);
        REQUIRE(std::get<1>(*(reg.query<Comp1>().begin() + 2)).value == 1);

        reg.destroy(entity);
        entity = reg.create();
        reg.emplace<Comp1>(entity, Comp1{4});
        REQUIRE(std::get<1>(*reg.query<Comp1>().begin()).value == 4);
        REQUIRE(std::get<1>(*(reg.query<Comp1>().begin() + 1)).value == 2);
        REQUIRE(std::get<1>(*(reg.query<Comp1>().begin() + 2)).value == 3);
    }

    SECTION("test") {
        auto entity0 = reg.create(); // 0
        auto entity1 = reg.create(); // 1
        auto entity2 = reg.create(); // 2
        auto entity3 = reg.create(); // 3
        auto entity4 = reg.create(); // 4
        auto entity5 = reg.create(); // 5
        auto entity6 = reg.create(); // 6

        reg.destroy(entity2);
        auto entity1048578 = reg.create();
        reg.destroy(entity3);
        auto entity1048579 = reg.create();
        reg.destroy(entity4);
        reg.destroy(entity5);
        auto entity1048580 = reg.create();
        reg.destroy(entity6);
        auto lastEntity = reg.create();
    }
}
