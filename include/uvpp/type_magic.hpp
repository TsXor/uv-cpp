#pragma once
#ifndef __UVPP_CORO_TYPE_MAGIC__
#define __UVPP_CORO_TYPE_MAGIC__

#include <tuple>
#include <variant>
#include <coroutine>
#include <uv.h>
#include "error.hpp"

namespace uvpp::utils {

template <typename T>
concept uv_request =
    std::is_same_v<T, uv_req_t> ||
    std::is_same_v<T, uv_getaddrinfo_t> ||
    std::is_same_v<T, uv_getnameinfo_t> ||
    std::is_same_v<T, uv_shutdown_t> ||
    std::is_same_v<T, uv_write_t> ||
    std::is_same_v<T, uv_connect_t> ||
    std::is_same_v<T, uv_udp_send_t> ||
    std::is_same_v<T, uv_fs_t> ||
    std::is_same_v<T, uv_work_t> ||
    std::is_same_v<T, uv_random_t>;

template <typename T>
concept uv_handle =
    std::is_same_v<T, uv_loop_t> ||
    std::is_same_v<T, uv_handle_t> ||
    std::is_same_v<T, uv_dir_t> ||
    std::is_same_v<T, uv_stream_t> ||
    std::is_same_v<T, uv_tcp_t> ||
    std::is_same_v<T, uv_udp_t> ||
    std::is_same_v<T, uv_pipe_t> ||
    std::is_same_v<T, uv_tty_t> ||
    std::is_same_v<T, uv_poll_t> ||
    std::is_same_v<T, uv_timer_t> ||
    std::is_same_v<T, uv_prepare_t> ||
    std::is_same_v<T, uv_check_t> ||
    std::is_same_v<T, uv_idle_t> ||
    std::is_same_v<T, uv_async_t> ||
    std::is_same_v<T, uv_process_t> ||
    std::is_same_v<T, uv_fs_event_t> ||
    std::is_same_v<T, uv_fs_poll_t> ||
    std::is_same_v<T, uv_signal_t>;


// https://zhuanlan.zhihu.com/p/102754882

template <typename... Ts>
struct param_pack_last;

template <typename T>
struct param_pack_last<T> { using value = T; };

template <typename T, typename... Ts>
struct param_pack_last<T, Ts...> {
    using value = typename param_pack_last<Ts...>::value;
};

} // namespace uvpp::utils


namespace uvco {

// what is done runtime:
// 1. Copy the libuv API function's parameters and function pointer
//    into a struct uv_async_api_data inside the awaitable.
// 2. Call the libuv API function with stored parameters when you
//    co_await the awaitable.
// 3. After some time, libuv eventloop executes callback_helper::call.
//    Callback parameters will be copied into a tuple inside the awaitable
//    and then your coroutine is resumed. Note that the first parameter,
//    which is the request or handle, is not copied.
// 4. The tuple inside the awaitable will become the result of the co_await
//    expression and returned to you.


// declare coro_wrap so that callback_helper can cast to it
template <typename FuncT>
struct coro_wrap;

namespace detail {

template <typename CallbackT, typename FuncT>
struct callback_helper;

template <typename ReqT, typename... Ts, typename FuncT>
struct callback_helper<void (*)(ReqT, Ts...), FuncT> {
    using return_type = std::tuple<Ts...>;
    static inline void call(ReqT request, Ts... params) {
        auto awaitable = reinterpret_cast<coro_wrap<FuncT>*>(request->data);
        awaitable->set_return(params...);
        awaitable->caller.resume();
    }
};


template <typename FuncT>
struct uv_async_api_trait;

// trait for event loop dependent APIs
template <typename ReqT, typename... ParamTs>
struct uv_async_api_trait<int(*)(uv_loop_t*, ReqT, ParamTs...)> {
    using callback_type = uvpp::utils::param_pack_last<ParamTs...>::value;
    using param_storage_type = std::tuple<uv_loop_t*, ReqT, ParamTs...>;
    static void*& get_data(param_storage_type& params) { return std::get<1>(params)->data; }
};

// trait for event loop independent APIs
template <typename ReqT, typename... ParamTs>
struct uv_async_api_trait<int(*)(ReqT, ParamTs...)> {
    using callback_type = uvpp::utils::param_pack_last<ParamTs...>::value;
    using param_storage_type = std::tuple<ReqT, ParamTs...>;
    static void*& get_data(param_storage_type& params) { return std::get<0>(params)->data; }
};

template <typename FuncT>
struct uv_async_api_data {
    using trait = uv_async_api_trait<FuncT>;
    using helper_type = callback_helper<typename trait::callback_type, FuncT>;
    
    FuncT func;
    trait::param_storage_type params;
    
    template <typename... Ts>
    uv_async_api_data(FuncT fn, Ts... rparams): func(fn),
        params(rparams..., helper_type::call) {}
    int call() { return std::apply(func, params); }
    void*& data() { return trait::get_data(params); }
};

} // namespace detail

template <typename FuncT>
struct coro_wrap {
    using api_type = detail::uv_async_api_data<FuncT>;
    using return_type = api_type::helper_type::return_type;
    using caller_type = std::coroutine_handle<>;

    caller_type caller;
    // These 2 things both need this Awaitable to store,
    // but not at the same time.
    std::variant<api_type, return_type> data;

    template <typename... Ts>
    coro_wrap(FuncT fn, Ts... rparams):
        data(std::in_place_type<api_type>, fn, rparams...) {}

    constexpr bool await_ready() const noexcept { return false; }
    void await_suspend(std::coroutine_handle<> ch) {
        caller = ch;
        auto& api = std::get<api_type>(data);
        api.data() = this;
        int status_code = api.call();
        if (status_code < 0) {
            throw uvpp::api_error(api.func, status_code);
        }
    }
    return_type await_resume() noexcept {
        return std::move(std::get<return_type>(data));
    }

    template <typename... Ts>
    void set_return(Ts... params) {
        data.template emplace<return_type>(params...);
    }
};

} // namespace uvco

#endif // __UVPP_CORO_TYPE_MAGIC__
