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

struct Event {
    int value;
};

void update_system(commands cmds, querier<Name> querier, resource<Res> res, event_dispatcher<Event> dispatcher) {
    auto it = querier.begin();
    REQUIRE(std::get<1>(*it).name == "ent2-trigged");
    it ++;
    REQUIRE(std::get<1>(*it).name == "ent1-trigged");
    REQUIRE(it + 1 == querier.end());

    REQUIRE(res->value == 123);
}

void trigger(entity entity, Name& name) {
    name.name += "-trigged";
}

void f(commands cmds) {
    auto entity1 = cmds.create();
    cmds.emplace<Name>(entity1, Name{"ent1"});
    auto entity2 = cmds.create();
    cmds.emplace<Name>(entity2, Name{"ent2"});

    cmds.emplace_resource<Res>(Res{123});
}

TEST_CASE("gecs") {
    world world;

    delegate<void(entity, Name&)> d;
    d.connect<trigger>();

    world.on_construct<Name>().add(d);
    world.regist_startup_system<f>();

    // use normal function
    world.regist_update_system<update_system>();

    world.startup();
    world.update();
}

