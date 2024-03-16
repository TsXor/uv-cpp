#pragma once
#ifndef __UVPP_STRUCT_WRAP_LOOP__
#define __UVPP_STRUCT_WRAP_LOOP__

#include <uv.h>
#include "uvpp/macros.hpp"
#include "wrapper_mixin.hpp"

namespace uvpp {


// loop_view is just a pointer with C++ sugar

struct loop_view : convertible_pointer_wrapper_mixin<loop_view, uv_loop_t> {
    uv_loop_t* ptr;
    loop_view() : ptr(nullptr) {}
    loop_view(uv_loop_t* ptr_) : ptr(ptr_) {}
    static size_t size() { return uv_loop_size(); }
    int alive() const { return uv_loop_alive(ptr); }
    template <typename... Ts> int configure(uv_loop_option option, Ts... params)
        { return uv_loop_configure(ptr, option, params...); }
    int fork() { return uv_loop_fork(ptr); }
    int run(uv_run_mode mode = UV_RUN_DEFAULT) { return uv_run(ptr, mode); }
    void stop() { uv_stop(ptr); }
};

// loop inherits loop_view, and manages the loop resource associated with the pointer

struct loop : loop_view {
    loop() : loop_view(new uv_loop_t()) {
        int status = uv_loop_init(ptr);
        if (status < 0) { throw uvpp::api_error(uv_loop_init, status); }
    }
    loop(const loop&) = delete;
    loop(loop&& other) { ptr = other.ptr; other.ptr = nullptr; }
    ~loop() { uv_loop_close(ptr); delete ptr; }
};

UVPP_FN loop_view default_loop() { return { uv_default_loop() }; }

UVPP_FN void close_default_loop_atexit() {
    std::atexit([](){ uv_loop_close(uv_default_loop()); });
}


} // namespace uvpp

# endif // __UVPP_STRUCT_WRAP_LOOP__
