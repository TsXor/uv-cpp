#include <stdio.h>
#include <coutils.hpp>
#include <uvpp.hpp>


constexpr int stdin_fd = 0;

coutils::async_fn<void> crunch_away(uv_idle_t* handle) {
    co_await uvco::watchers::start(handle);
    // - compute extra-terrestrial life
    // - fold proteins
    // - compute another digit of PI
    // or similar ...
    fprintf(stderr, "Computing PI...\n");
    // just to avoid overwhelming your terminal emulator
    uvco::watchers::stop(handle);
}

coutils::async_fn<void> watch_stdin(uv_loop_t* loop) {
    uvpp::watchers::idle idler(loop);
    uvpp::fs::request read_req;
    char buffer[1024]; uvpp::buf_view bufv(buffer, 1024);
    while (true) {
        try {
            co_await coutils::all_completed(
                uvco::fs::read(loop, read_req, stdin_fd, &bufv, 1, -1),
                crunch_away(idler)
            );
        } catch (const uvpp::api_error& err) {
            fprintf(stderr, "error opening file: %s\n", err.str());
            co_return;
        }
        int input_size = read_req->result;
        buffer[input_size] = '\0';
        // strip out trailing whitespace
        while (input_size > 0) {
            if (isspace(buffer[input_size - 1])) {
                buffer[input_size - 1] = '\0';
                input_size--;
            } else {
                break;
            }
        }
        if (input_size == 0) {
            printf("Empty input, exiting ...\n");
            break;
        }
        printf("Typed %s\n", buffer);
    }
}

int main() {
    auto loop = uvpp::default_loop();
    uvpp::close_default_loop_atexit();
    coutils::sync::unleash(watch_stdin(loop));
    return loop.run();
}
