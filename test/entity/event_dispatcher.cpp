#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "gecs/entity/event_dispatcher.hpp"
#include "gecs/gecs.hpp"

struct Event {
    int value;
};

int gCount = 0;

void Foo(const Event& event) {
    gCount += event.value;
}

TEST_CASE("event dispatcher") {
    gecs::world w;
    auto& reg = w.regist_registry("reg");

    gecs::basic_event_dispatcher<Event, typename gecs::world::registry_type> dispatcher(reg);
    auto sink = dispatcher.sink();
    sink.add<Foo>();
    dispatcher.trigger(Event{1});
    REQUIRE(gCount == 1);

    sink.add<Foo>();
    dispatcher.trigger(Event{2});
    REQUIRE(gCount == 5);

    dispatcher.enqueue(Event{3});
    dispatcher.enqueue(Event{4});
    dispatcher.enqueue(Event{5});
    dispatcher.update();
    REQUIRE(gCount == 29);
}