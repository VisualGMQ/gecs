#include "gecs/config/config.hpp"
#include "gecs/entity/sparse_set.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace gecs;

#define Entity(entity) static_cast<config::Entity>(entity)

TEST_CASE("utilities") {
    REQUIRE(internal::popcount(0b1111) == 4);
    REQUIRE(internal::popcount(0b111111111111) == 12);
    REQUIRE(internal::popcount(0x0) == 0);
    REQUIRE(internal::popcount(0b001011) == 3);

    REQUIRE(internal::entity_to_integral<config::Entity>(Entity(3)) == 3);
    REQUIRE(internal::entity_id<config::Entity>(Entity(3)) == 3);
    REQUIRE(internal::entity_id<config::Entity>(Entity(10086)) == 10086);

    auto entity = internal::construct_entity<config::Entity>(2, 1);
    REQUIRE(internal::entity_version<config::Entity>(entity) == 2);
    REQUIRE(internal::entity_id<config::Entity>(entity) == 1);

    REQUIRE(internal::entity_next_version(entity) == 3);
    auto next_entity = internal::entity_inc_version(entity);
    REQUIRE(internal::entity_version<config::Entity>(next_entity) == 3);
    REQUIRE(internal::entity_id<config::Entity>(next_entity) == 1);

    auto entity2 = internal::construct_entity<config::Entity>(0, 4);
    REQUIRE(internal::combine_entity(next_entity, entity2) == internal::construct_entity<config::Entity>(3, 4));
}