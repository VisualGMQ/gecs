#include "gecs/config.hpp"
#include "gecs/sparse_set.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace gecs;

#define Entity(x) static_cast<config::Entity>(x)

TEST_CASE("basic sparse set") {
    basic_sparse_set<config::Entity, config::PageSize> set;

    SECTION("insert") {
        set.insert(Entity(2));
        REQUIRE(set.size() == 1);

        set.insert(Entity(4));
        REQUIRE(set.size() == 2);

        REQUIRE(set.contain(Entity(2)));
        REQUIRE(set.contain(Entity(4)));
        REQUIRE_FALSE(set.contain(Entity(0)));
        REQUIRE_FALSE(set.contain(Entity(9)));

        set.remove(Entity(2));
        REQUIRE_FALSE(set.contain(Entity(2)));
        REQUIRE(set.size() == 1);
        REQUIRE(!set.empty());

        set.clear();
        REQUIRE(set.size() == 0);
        REQUIRE(set.empty());
    }
}