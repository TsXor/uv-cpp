#include <stdio.h>
#include <thread>
#include <chrono>
#include <coutils.hpp>
#include <uvpp.hpp>


coutils::async_fn<void> wait_for_a_while(uv_loop_t* loop) {
    uvpp::watchers::idle idler(loop);
    int64_t counter = 0;
    printf("Start idling...\n");
    co_await uvco::watchers::start(idler);
    while (counter < 10e6) {
        counter++;
        co_await uvco::watchers::next_loop();
    }
    uvco::watchers::stop(idler);
}

int main() {
    auto loop = uvpp::default_loop();
    uvpp::close_default_loop_atexit();

    auto wait_task = coutils::sync::controlled_of(wait_for_a_while(loop));
    std::thread loop_thread([&](){ loop.run(); });
    
    wait_task.start();
    while (!wait_task.completed) {
        printf("Main thread spinning...\n");
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    printf("Main thread finished waiting.\n");

    loop_thread.join();
    return 0;
}
