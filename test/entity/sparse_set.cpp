#include "gecs/config/config.hpp"
#include "gecs/entity/sparse_set.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace gecs;

#define Entity(x) static_cast<config::Entity>(x)

using sparse_set = basic_sparse_set<config::Entity, config::PageSize>;

TEST_CASE("functionalities") {
    sparse_set set;

    SECTION("initial state") {
        REQUIRE(set.empty());
        REQUIRE(set.size() == 0);
        REQUIRE(set.capacity() == 0);
        REQUIRE(set.begin() == set.end());
        REQUIRE(set.cbegin() == set.cend());
        REQUIRE(set.crbegin() == set.crend());
    }

    SECTION("reserve") {
        set.reserve(4);
        REQUIRE(set.size() == 0);
        REQUIRE(set.empty());
        REQUIRE(set.begin() == set.end());
        REQUIRE(set.cbegin() == set.cend());
        REQUIRE(set.crbegin() == set.crend());
    }

    SECTION("iterator") {
        REQUIRE(set.begin() == set.end());
        set.insert(Entity(8));
        set.insert(Entity(9));
        set.insert(Entity(3));
        set.insert(Entity(2));

        auto begin = set.begin();
        REQUIRE_FALSE(begin == set.end());
        REQUIRE(begin != set.end());

        REQUIRE(*(begin++) == 2);
        REQUIRE(*(begin++) == 3);
        REQUIRE(*(begin++) == 9);
        REQUIRE(*(begin++) == 8);
        REQUIRE(begin == set.end());
        REQUIRE(*(--begin) == 8);
        REQUIRE(*(--begin) == 9);
        REQUIRE(*(--begin) == 3);
        REQUIRE(*(--begin) == 2);

        auto rbegin = set.rbegin();
        REQUIRE_FALSE(rbegin == set.rend());
        REQUIRE(*(rbegin++) == 8);
        REQUIRE(*(rbegin++) == 9);
        REQUIRE(*(rbegin++) == 3);
        REQUIRE(*(rbegin++) == 2);
        REQUIRE(rbegin == set.rend());

        begin = set.begin();
        REQUIRE(*(begin + 2) == 9);
    }
}

TEST_CASE("insert and remove") {
    sparse_set set;

    SECTION("insert") {
        set.insert(Entity(2));
        REQUIRE(set.size() == 1);

        set.insert(Entity(4));
        REQUIRE(set.size() == 2);

        REQUIRE(set.contain(Entity(2)));
        REQUIRE(set.contain(Entity(4)));
        REQUIRE_FALSE(set.contain(Entity(0)));
        REQUIRE_FALSE(set.contain(Entity(9)));
    }

    SECTION("remove") {
        set.remove(Entity(3));
        REQUIRE(!set.contain(Entity(3)));
        REQUIRE(set.size() == 0);
        REQUIRE(set.empty());
        REQUIRE(set.begin() == set.end());

        set.insert(Entity(2));
        set.insert(Entity(4));
        set.remove(Entity(2));
        REQUIRE_FALSE(set.contain(Entity(2)));
        REQUIRE(set.size() == 1);
        REQUIRE(!set.empty());

        set.remove(Entity(4));
        REQUIRE_FALSE(set.contain(Entity(4)));
        REQUIRE(set.size() == 0);
        REQUIRE(set.empty());

        set.insert(Entity(1));
        set.insert(Entity(2));

        set.clear();
        REQUIRE(set.size() == 0);
        REQUIRE(set.empty());
    }
}

TEST_CASE("pump") {
    sparse_set set;

    set.insert(Entity(1));
    REQUIRE(set.pump(Entity(1)) == 1);
    REQUIRE(set.size() == 1);
    REQUIRE(*(set.begin()) == 1);

    set.insert(Entity(2));
    REQUIRE(*set.begin() == 2);

    set.insert(Entity(3));
    REQUIRE(*set.begin() == 3);

    set.pump(Entity(2));
    REQUIRE(set.size() == 3);
    REQUIRE(*set.begin() == 2);
    REQUIRE(*(set.begin() + 1) == 3);
    REQUIRE(*(set.begin() + 2) == 1);
}


TEST_CASE("index") {
    sparse_set set;
    set.insert(Entity(2));
    set.insert(Entity(3));
    set.insert(Entity(4));
    REQUIRE(set.index(Entity(2)) == 0);
    REQUIRE(set.index(Entity(3)) == 1);
    REQUIRE(set.index(Entity(4)) == 2);

    set.remove(Entity(3));
    REQUIRE(set.index(Entity(2)) == 0);
    REQUIRE(set.index(Entity(4)) == 1);
}
