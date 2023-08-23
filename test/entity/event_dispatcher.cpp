#include "gecs/entity/event_dispatcher.hpp"
#include "gecs/gecs.hpp"

struct Event {};
struct Ctx {};

void Foo(const Event& event, gecs::commands cmds) {
}

int main(int argc, char** argv) {
    gecs::world w;

    gecs::basic_event_dispatcher<Event, gecs::world> dispatcher(w);
    auto sink = dispatcher.sink();
    sink.add<Foo>();
    dispatcher.trigger(Event{});
    return 0;
}