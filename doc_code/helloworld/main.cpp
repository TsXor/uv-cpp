#include <iostream>
#include <uvpp.hpp>

int main() {
    uvpp::loop loop;
    std::cout << "Now quitting.\n";
    loop.run();
    return 0;
}
