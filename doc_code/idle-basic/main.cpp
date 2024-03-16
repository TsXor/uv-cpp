#include <stdio.h>
#include <uvpp.hpp>


uvco::coro_task wait_for_a_while(uv_idle_t* handle) {
    int64_t counter = 0;
    co_await uvco::watchers::start(handle);
    while (counter < 10e6) {
        counter++;
        co_await uvco::watchers::next_loop();
    }
    uvco::watchers::stop(handle);
}

int main() {
    auto loop = uvpp::default_loop();
    uvpp::close_default_loop_atexit();

    uvpp::watchers::idle idler(loop);
    wait_for_a_while(idler);

    printf("Idling...\n");
    loop.run();

    return 0;
}
