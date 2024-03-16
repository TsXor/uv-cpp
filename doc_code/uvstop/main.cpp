#include <stdio.h>
#include <uvpp.hpp>


uvco::coro_task do_idle(uvpp::loop_view loop) {
    uvpp::watchers::idle handle(loop);
    int64_t counter = 0;
    co_await uvco::watchers::start(handle);
    while (true) {
        printf("Idle callback\n");
        counter++;
        if (counter >= 5) { break; }
        co_await uvco::watchers::next_loop();
    }
    loop.stop();
    printf("uv_stop() called\n");
    
}

uvco::coro_task do_prepare(uv_loop_t* loop) {
    uvpp::watchers::prepare handle(loop);
    co_await uvco::watchers::start(handle);
    printf("Prep callback\n");
}

int main() {
    auto loop = uvpp::default_loop();
    do_idle(loop);
    do_prepare(loop);
    loop.run();
    return 0;
}
