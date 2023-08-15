#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "gecs/entity/storage.hpp"
#include "gecs/config/config.hpp"

using namespace gecs;

#define Entity(x) static_cast<config::Entity>(x)

TEST_CASE("pure entity storage") {
    basic_storage<config::Entity, config::Entity, config::PageSize, void> storage;

    REQUIRE(storage.size() == 0);
    REQUIRE(storage.base_size() == 0);

    auto entity = storage.emplace();
    REQUIRE(entity == 0);
    REQUIRE(storage.size() == 1);
    REQUIRE(storage.base_size() == 1);
    entity = storage.emplace();
    REQUIRE(entity == 1);
    REQUIRE(storage.size() == 2);
    REQUIRE(storage.base_size() == 2);

    storage.remove(Entity(1));
    REQUIRE(storage.size() == 1);
    REQUIRE(storage.base_size() == 2);

    entity = storage.emplace();
    REQUIRE(entity == internal::construct_entity<config::Entity>(1, 1));
    REQUIRE(storage.size() == 2);
    REQUIRE(storage.base_size() == 2);

    REQUIRE_FALSE(storage.contain(Entity(1)));
    REQUIRE(storage.contain(entity));
}

struct Vector2 {
    float x, y;

    bool operator==(const Vector2& o) const {
        return x == o.x && y == o.y;
    }
};

TEST_CASE("storage") {
    basic_storage<config::Entity, Vector2, config::PageSize, std::allocator<Vector2>> storage;

    REQUIRE(storage.size() == 0);
    REQUIRE(storage.empty());
    REQUIRE_FALSE(storage.contain(Entity(2)));
    REQUIRE(storage.begin() == storage.end());

    auto& v = storage.emplace(Entity(0), Vector2{1.0, 3.0});
    REQUIRE(v.x == 1.0);
    REQUIRE(v.y == 3.0);
    REQUIRE(storage.size() == 1);
    REQUIRE(!storage.empty());
    REQUIRE(storage.contain(Entity(0)));
    REQUIRE_FALSE(storage.contain(Entity(2)));
    REQUIRE(storage.begin() != storage.end());
    REQUIRE((*storage.begin())->x == 1.0);
    REQUIRE((*storage.begin())->y == 3.0);

    auto& v2 = storage.insert(Entity(2), Vector2{4.0, 7.0});
    REQUIRE(v2.x == 4.0);
    REQUIRE(v2.y == 7.0);
    REQUIRE(storage.size() == 2);
    REQUIRE(*storage.begin() == &v2);
    REQUIRE(*(storage.begin() + 1) == &v);

    auto& v3 = storage.emplace(Entity(4), 8.0f, 9.0f);

    auto begin = storage.begin();
    REQUIRE(**begin == Vector2{8.0, 9.0});
    begin ++;
    REQUIRE(*begin == &v2);
    begin ++;
    REQUIRE(*begin == &v);
    begin ++;
    REQUIRE(begin == storage.end());

    storage.remove(Entity(2));
    REQUIRE_FALSE(storage.contain(Entity(2)));
    REQUIRE(**storage.find(Entity(4)) == v3);
    REQUIRE(**storage.find(Entity(0)) == v);
    REQUIRE(storage[Entity(4)] == v3);
    REQUIRE(storage[Entity(0)] == v);
    storage.remove(Entity(0));
    REQUIRE(**storage.find(Entity(4)) == v3);
    REQUIRE(storage.find(Entity(0)) == storage.end());
}
