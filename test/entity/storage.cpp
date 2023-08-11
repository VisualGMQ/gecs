#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "gecs/entity/storage.hpp"
#include "gecs/config/config.hpp"

using namespace gecs;

#define Entity(x) static_cast<config::Entity>(x)

TEST_CASE("pure entity storage") {
    basic_storage<config::Entity, config::Entity, config::PageSize> storage;

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
}

TEST_CASE("storage") {
}