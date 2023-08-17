#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "gecs/gecs.hpp"

using namespace gecs;

struct Name {
    std::string name;
};

struct Res {
    int value;
};

void update_system(commands cmds, querier<Name> querier, resource<Res> res) {
    auto it = querier.begin();
    REQUIRE(std::get<1>(*it).name == "ent2");
    it ++;
    REQUIRE(std::get<1>(*it).name == "ent1");
    REQUIRE(it + 1 == querier.end());

    REQUIRE(res->value == 123);
}

TEST_CASE("gecs") {
    world world;

    // use non-capture lambda
    world.regist_startup_system<+[](commands cmds) {
        auto entity1 = cmds.create();
        cmds.emplace<Name>(entity1, Name{"ent1"});
        auto entity2 = cmds.create();
        cmds.emplace<Name>(entity2, Name{"ent2"});

        cmds.emplace_resource<Res>(Res{123});
    }>();

    // use normal function
    world.regist_update_system<update_system>();

    world.startup();
    world.update();
}

