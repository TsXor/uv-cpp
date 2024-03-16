#pragma once
#ifndef __UVPP_COROUTINE_UTILS__
#define __UVPP_COROUTINE_UTILS__


#include <atomic>
#include <coroutine>
#include <type_traits>
#include <utility>


namespace uvco {


// simple coroutine return type, not awaitable
struct coro_task {
    struct promise_type {
        coro_task get_return_object() { return {}; }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };
};

// transfer control to other handle
struct switch_to_other {
    std::coroutine_handle<> other;
    constexpr bool await_ready() const noexcept { return false; }
    constexpr void await_resume() const noexcept {}
    auto await_suspend(std::coroutine_handle<> ch) const noexcept { return other; }
};

// wraps coroutine like an async function
// won't be executed unless you co_await it
template <typename T>
struct coro_fn {
    struct promise_type;
    using callee_type = std::coroutine_handle<promise_type>;
    callee_type callee;
    T retval;
    
    constexpr bool await_ready() const noexcept { return false; }
    
    coro_fn(promise_type& callee_promise):
        callee(callee_type::from_promise(callee_promise)) {}
    auto await_suspend(std::coroutine_handle<> ch) {
        callee.promise().caller = ch;
        callee.promise().retval_ptr = &retval;
        return callee;
    }
    T await_resume() {
        return std::move(retval);
    }

    struct promise_type {
        std::coroutine_handle<> caller;
        T* retval_ptr = nullptr;
        coro_fn get_return_object() { return { *this }; }
        void return_value(T&& ret) { if (retval_ptr) *retval_ptr = std::move(ret); }
        std::suspend_always initial_suspend() { return {}; }
        switch_to_other final_suspend() noexcept { return { caller }; }
        void unhandled_exception() {}
    };
};

// no return value specialization
template <>
struct coro_fn<void> {
    struct promise_type;
    using callee_type = std::coroutine_handle<promise_type>;
    callee_type callee;
    
    constexpr bool await_ready() const noexcept { return false; }
    
    coro_fn(promise_type& callee_promise):
        callee(callee_type::from_promise(callee_promise)) {}
    auto await_suspend(std::coroutine_handle<> ch) {
        callee.promise().caller = ch;
        return callee;
    }
    void await_resume() {}

    struct promise_type {
        std::coroutine_handle<> caller;
        coro_fn get_return_object() { return { *this }; }
        void return_void() {}
        std::suspend_always initial_suspend() { return {}; }
        switch_to_other final_suspend() noexcept { return { caller }; }
        void unhandled_exception() {}
    };
};

struct empty_slot_t { explicit empty_slot_t() = default; };
static inline constexpr auto empty_slot = empty_slot_t();

namespace detail {

coro_fn<void> barrier(size_t n) {
    size_t count = 0;
    while (true) {
        co_await std::suspend_always{};
        if (++count >= n) break;
    }
}

coro_fn<void> tsafe_barrier(size_t n) {
    std::atomic<size_t> count = 0;
    while (true) {
        co_await std::suspend_always{};
        int local_count = ++count;
        if (local_count >= n) break;
    }
}

template <typename T>
struct replace_empty_type {
    static constexpr bool is_empty = false;
    using value = T;
};
template <>
struct replace_empty_type<void> {
    static constexpr bool is_empty = true;
    using value = empty_slot_t;
};
template <>
struct replace_empty_type<std::tuple<>> {
    static constexpr bool is_empty = true;
    using value = empty_slot_t;
};

template <typename T>
struct awaitable_trait {
    using eval_type = std::invoke_result<decltype(T::await_resume), T*>::type;
    using suspend_return_type = std::invoke_result<decltype(T::await_suspend), T*, std::coroutine_handle<>>::type;
    using result_type = replace_empty_type<eval_type>::value;
    
    static constexpr bool is_void_suspend = std::is_same_v<suspend_return_type, void>;
    static constexpr bool is_bool_suspend = std::is_same_v<suspend_return_type, bool>;
    static constexpr bool is_handle_suspend = std::is_convertible_v<suspend_return_type, std::coroutine_handle<>>;
    
    static bool start(T& waiter, std::coroutine_handle<> ch) requires is_void_suspend {
        bool ready = waiter.await_ready();
        if (ready) { return false; }
        waiter.await_suspend(ch);
        return true;
    }
    static bool start(T& waiter, std::coroutine_handle<> ch) requires is_bool_suspend {
        bool ready = waiter.await_ready();
        if (ready) { return false; }
        bool suspended = waiter.await_suspend(ch);
        return suspended;
    }
    static bool start(T& waiter, std::coroutine_handle<> ch) requires is_handle_suspend {
        bool ready = waiter.await_ready();
        if (ready) { return false; }
        std::coroutine_handle<> hd = waiter.await_suspend(ch);
        hd.resume(); return true;
    }

    static auto get_return(T& waiter) {
        if constexpr (replace_empty_type<eval_type>::is_empty) {
            return empty_slot;
        } else {
            return waiter.await_resume();
        }
    }
};

template <typename... AwaitableTs>
using awaitable_merge_result = std::tuple<typename awaitable_trait<AwaitableTs>::result_type ...>;

template <typename... AwaitableTs>
using merge_fn = coro_fn<awaitable_merge_result<AwaitableTs...>>;

} // namespace detail

// suspend current coroutine and resume after all given awaitables are completed
// the awaitables will be started SERIALLY on the SAME THREAD as caller
//
// for awaitables that may cause coroutines to be continued on other threads,
// argument thread_safe should be set to true to use a thread-safe barrier 
// 
// the result of co_await-ing this is the results of all given awaitables packed
// in a tuple (in given order), where empty results (void and std::tuple<>) are
// replaced with a marker object uvco::empty_slot
template <typename... AwaitableTs>
detail::merge_fn<AwaitableTs...> all_completed(bool thread_safe, AwaitableTs&&... awaitables) {
    static_assert(sizeof...(awaitables) > 0, "you should wait for something");
    coro_fn<void> barrier_waiter = thread_safe
        ? detail::tsafe_barrier(sizeof...(awaitables) + 1)
        : detail::barrier(sizeof...(awaitables) + 1);
    barrier_waiter.callee.resume(); // coro_fn is lazy, start it
    auto check_dec_barrier = [&](bool suspended) { if (!suspended) { barrier_waiter.callee.resume(); } };
    (..., check_dec_barrier(detail::awaitable_trait<AwaitableTs>::start(awaitables, barrier_waiter.callee)));
    co_await barrier_waiter;
    co_return std::tuple(detail::awaitable_trait<AwaitableTs>::get_return(awaitables)...);
}


} // namespace uvco


#endif // __UVPP_COROUTINE_UTILS__
