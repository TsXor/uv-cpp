#pragma once
#ifndef __UVPP_COROUTINE_WATCHERS__
#define __UVPP_COROUTINE_WATCHERS__

#include <coroutine>
#include <uv.h>
#include "uvpp/type_magic.hpp"
#include "uvpp/macros.hpp"
#include "uvpp/coro_utils.hpp"

namespace uvco::watchers {

#define UVPP_LOOP_WATCHER_FUNC_DEFINE(name) \
    UVPP_FN auto start(uv_##name##_t* handle) { return coro_wrap(uv_##name##_start, handle); } \
    UVPP_FN void stop(uv_##name##_t* handle) { uv_##name##_stop(handle); }

UVPP_LOOP_WATCHER_FUNC_DEFINE(prepare)
UVPP_LOOP_WATCHER_FUNC_DEFINE(check)
UVPP_LOOP_WATCHER_FUNC_DEFINE(idle)

// returns std::suspend_always, just a semantic shortcut
UVPP_FN std::suspend_always next_loop() { return {}; }

} // namespace uvco::watchers

namespace uvco::utils {

// suspend execution and resume at next loop
UVPP_FN coro_fn<void> interrupt(uv_idle_t* idler) {
    co_await watchers::start(idler);
    watchers::stop(idler);
}

// suspend execution and resume after N loops
UVPP_FN coro_fn<void> sleep(uv_idle_t* idler, size_t n_loops) {
    size_t past_loops = 0;
    co_await watchers::start(idler);
    while (past_loops < n_loops) {
        past_loops++;
        co_await watchers::next_loop();
    }
    watchers::stop(idler);
}

} // namespace uvco::utils

# endif // __UVPP_COROUTINE_WATCHERS__
