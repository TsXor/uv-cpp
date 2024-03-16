#pragma once
#ifndef __UVPP_STRUCT_WRAP_WATCHER__
#define __UVPP_STRUCT_WRAP_WATCHER__

#include <functional>
#include <uv.h>
#include "uvpp/macros.hpp"
#include "wrapper_mixin.hpp"

namespace uvpp::watchers {

#define UVPP_LOOP_WATCHER_STRUCT_DEFINE(name) \
    struct name : convertible_pointer_wrapper_mixin<name, uv_##name##_t> { \
        uv_##name##_t* ptr; \
        name(uv_loop_t* loop) : ptr(new uv_##name##_t()) { \
            int status = uv_##name##_init(loop, ptr); \
            if (status < 0) { throw uvpp::api_error(uv_##name##_init, status); } \
        } \
        ~name() { delete ptr; } \
    };

UVPP_LOOP_WATCHER_STRUCT_DEFINE(prepare)
UVPP_LOOP_WATCHER_STRUCT_DEFINE(check)
UVPP_LOOP_WATCHER_STRUCT_DEFINE(idle)

} // namespace uvpp::watchers

# endif // __UVPP_STRUCT_WRAP_WATCHER__
