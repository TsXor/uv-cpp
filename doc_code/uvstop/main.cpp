#include <stdio.h>
#include <coutils.hpp>
#include <uvpp.hpp>


coutils::async_fn<void> do_idle(uvpp::loop_view loop) {
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

coutils::async_fn<void> do_prepare(uv_loop_t* loop) {
    uvpp::watchers::prepare handle(loop);
    co_await uvco::watchers::start(handle);
    printf("Prep callback\n");
}

int main() {
    auto loop = uvpp::default_loop();
    coutils::sync::unleash(do_idle(loop));
    coutils::sync::unleash(do_prepare(loop));
    loop.run();
    return 0;
}
