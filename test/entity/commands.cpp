#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "gecs/gecs.hpp"

using namespace gecs;

gecs::world w;

struct Comp1 {
    float value;
};

struct Comp2 {
    int value;
};

struct CompBundle {
    Comp1 comp1;
    Comp2 comp2;
};

TEST_CASE("commands") {
    w.regist_registry("gaming");
    w.startup();
    auto& reg = *w.cur_registry();
    auto cmds = reg.commands();

    SECTION("create entity") {
        auto entity = cmds.create();

        REQUIRE(reg.alive(entity));
        REQUIRE(reg.entities().size() == 1);

        entity = cmds.create();
        REQUIRE(reg.alive(entity));
        REQUIRE(reg.entities().size() == 2);

        cmds.destroy(entity);
        REQUIRE_FALSE(reg.alive(entity));
        REQUIRE(reg.entities().size() == 1);
    }

    SECTION("create component") {
        auto ent = cmds.create();
        cmds.emplace<Comp1>(ent, Comp1{123.0});

        REQUIRE(reg.has<Comp1>(ent));
        REQUIRE(reg.get<Comp1>(ent).value == 123);

        cmds.remove<Comp1>(ent);
        REQUIRE_FALSE(reg.has<Comp1>(ent));
    }

    SECTION("create bundle") {
        auto ent = cmds.create();
        cmds.emplace_bundle(ent, CompBundle{Comp1{123.0}, Comp2{54}});

        REQUIRE(reg.has<Comp1>(ent));
        REQUIRE(reg.has<Comp2>(ent));
        REQUIRE(reg.get<Comp1>(ent).value == 123);
        REQUIRE(reg.get<Comp2>(ent).value == 54);
    }
}