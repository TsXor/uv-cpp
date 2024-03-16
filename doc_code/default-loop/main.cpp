#include <stdio.h>
#include <uvpp.hpp>

int main() {
    auto loop = uvpp::default_loop();
    // uvpp::default_loop() returns a loop_view
    // which does not manage the lifecycle of default loop
    // so you may need to manually close it
    uvpp::close_default_loop_atexit();

    printf("Default loop.\n");
    loop.run();

    return 0;
}
