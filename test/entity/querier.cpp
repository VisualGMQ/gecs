#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "gecs/entity/querier.hpp"
#include "gecs/entity/registry.hpp"

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

struct Comp4 {
    bool b;
};

using trivial_owner = int;
using registry = gecs::basic_registry<trivial_owner, gecs::config::Entity, gecs::config::PageSize>;

trivial_owner owner;

TEST_CASE("querier misc") {
    registry reg(owner);

    SECTION("init state check") {
        auto querier = reg.query<Comp1>();
        REQUIRE(querier.begin() == querier.end());
        REQUIRE(querier.size() == 0);
    }
}

TEST_CASE("single querier") {
    registry reg(owner);

    for (int i = 0; i < 5; i++) {
        auto entity = reg.create();
        reg.emplace<Comp1>(entity, Comp1{i});
    }
    
    SECTION("non-mutable single query") {
        auto querier = reg.query<Comp1>();
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
        auto querier = reg.query<mut<Comp1>>();
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
    registry reg(owner);

    std::array<registry::entity_type, 5> entities = {
        reg.create(),
        reg.create(),
        reg.create(),
        reg.create(),
        reg.create(),
    };

    reg.emplace<Comp1>(entities[0], Comp1{8});
    reg.emplace<Comp1>(entities[1], Comp1{2});
    reg.emplace<Comp1>(entities[2], Comp1{0});
    reg.emplace<Comp1>(entities[3], Comp1{9});
    reg.emplace<Comp2>(entities[1], Comp2{"ent2"});
    reg.emplace<Comp2>(entities[4], Comp2{"ent3"});
    reg.emplace<Comp4>(entities[0], Comp4{true});

    SECTION("non-mutable query") {
        auto querier = reg.query<Comp1, Comp2>();
        auto begin = querier.begin();

        REQUIRE(querier.size() == 1);
        REQUIRE(std::get<1>(*begin).value == 2);
        REQUIRE(std::get<2>(*begin).name == "ent2");
        REQUIRE(begin + 1 == querier.end());
    }

    SECTION("invalid component query") {
        auto querier = reg.query<Comp3>();

        REQUIRE(querier.size() == 0);
        REQUIRE(querier.begin() == querier.end());
    }

    SECTION("sort") {
        auto querier = reg.query<Comp1>().sort_by<Comp1>([](const Comp1& c1, const Comp1& c2){
            return c1.value < c2.value;
        });
        auto it = querier.begin();
        REQUIRE(std::get<1>(*it).value == 0);
        it ++;
        REQUIRE(std::get<1>(*it).value == 2);
        it ++;
        REQUIRE(std::get<1>(*it).value == 8);
        it ++;
        REQUIRE(std::get<1>(*it).value == 9);
    }

    SECTION("`without` query", "[conditional]") {
        auto querier = reg.query<Comp1, without<Comp2>>();
        REQUIRE(querier.size() == 3);

        auto it = querier.begin();
        REQUIRE(std::get<1>(*it).value == 9);
        it++;
        REQUIRE(std::get<1>(*it).value == 0);
        it++;
        REQUIRE(std::get<1>(*it).value == 8);

        auto querier3 = reg.query<Comp2, without<Comp1, Comp3>>();
        REQUIRE(querier3.size() == 1);
        auto it3 = querier3.begin();
        REQUIRE(std::get<1>(*it3).name == "ent3");
    }

    SECTION("`only` query", "[conditional]") {
        auto querier = reg.query<only<Comp1>>();
        REQUIRE(querier.size() == 2);

        auto it = querier.begin();
        REQUIRE(std::get<1>(*it).value == 9);
        it++;
        REQUIRE(std::get<1>(*it).value == 0);

        auto querier2 = reg.query<only<Comp1, Comp2>>();
        REQUIRE(querier2.size() == 1);
        REQUIRE(std::get<1>(*querier2.begin()).value == 2);
    }

    SECTION("mutable query") {
        auto querier = reg.query<mut<Comp1>, mut<Comp2>>();

        REQUIRE(querier.size() == 1);
    }
}
