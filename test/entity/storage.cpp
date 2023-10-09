#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "gecs/entity/storage.hpp"
#include "gecs/config/config.hpp"

using namespace gecs;

#define Entity(x) static_cast<config::Entity>(x)

TEST_CASE("pure entity storage") {
    basic_storage<config::Entity, config::Entity, config::PageSize, void, void> storage;

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

struct Value {
    int value;
};

TEST_CASE("storage") {
    basic_storage<config::Entity, Vector2, config::PageSize, std::allocator<Vector2>, config::type_info> storage;

    SECTION("misc") {
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

    SECTION("sort") {
        basic_storage<config::Entity, Value, config::PageSize, std::allocator<Value>, config::type_info> storage;
        storage.emplace<Value>(Entity(0), Value{2});
        storage.emplace<Value>(Entity(1), Value{8});
        storage.emplace<Value>(Entity(2), Value{5});
        storage.emplace<Value>(Entity(3), Value{1});
        storage.emplace<Value>(Entity(4), Value{0});

        storage.sort(storage.begin(), storage.end(), [](const Value& v1, const Value& v2){
            return v1.value < v2.value;
        });

        REQUIRE(storage.payloads()[4]->value == 0);
        REQUIRE(storage.payloads()[3]->value == 1);
        REQUIRE(storage.payloads()[2]->value == 2);
        REQUIRE(storage.payloads()[1]->value == 5);
        REQUIRE(storage.payloads()[0]->value == 8);

        REQUIRE(storage.packed()[4] == 4);
        REQUIRE(storage.packed()[3] == 3);
        REQUIRE(storage.packed()[2] == 0);
        REQUIRE(storage.packed()[1] == 2);
        REQUIRE(storage.packed()[0] == 1);
    }
}
