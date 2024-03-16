#pragma once
#ifndef __UVPP_STRUCT_WRAP_REQUESTS__
#define __UVPP_STRUCT_WRAP_REQUESTS__

#include <uv.h>
#include "uvpp/macros.hpp"
#include "wrapper_mixin.hpp"
#include "uvpp/type_magic.hpp"


// base request method overload for subclasses

template <uvpp::utils::uv_request ReqT>
    requires (!std::is_same_v<ReqT, uv_req_t>)
uv_req_type uv_req_get_type(const ReqT* req) {
    return uv_req_get_type(reinterpret_cast<uv_req_t*>(req));
}
template <uvpp::utils::uv_request ReqT>
    requires (!std::is_same_v<ReqT, uv_req_t>)
void* uv_req_get_data(const ReqT* req) {
    return uv_req_get_data(reinterpret_cast<uv_req_t*>(req));
}
template <uvpp::utils::uv_request ReqT>
    requires (!std::is_same_v<ReqT, uv_req_t>)
void uv_req_set_data(ReqT* req, void* data) {
    return uv_req_set_data(reinterpret_cast<uv_req_t*>(req), data);
}


namespace uvpp {


namespace detail {

UVPP_FN void dealloc_request(void* ptr) {
    // specific cleanup
    switch (((uv_req_t*)ptr)->type) {
        case UV_FS: uv_fs_req_cleanup((uv_fs_t*)ptr);
    }
    // delete
    switch (((uv_req_t*)ptr)->type) {
#define __CASE_DELETE_UV_STRUCT(enum_name, type_name) case UV_##enum_name: delete (uv_##type_name##_t*)ptr; break;
        UV_REQ_TYPE_MAP(__CASE_DELETE_UV_STRUCT)
#undef __CASE_DELETE_UV_STRUCT
        default: delete (uv_req_t*)ptr;
    }
}

template <typename Derived>
struct base_request_mixin {
    int cancel() { __UVPP_CRTP_CAST(Derived); return uv_cancel(sub->ptr); }
    static size_t size(uv_req_type type) { return uv_req_size(type); }
    void* get_data() const { __UVPP_CRTP_CAST_CONST(Derived); return uv_req_get_data(sub->ptr); }
    void set_data(void* data) { __UVPP_CRTP_CAST(Derived); return uv_req_set_data(sub->ptr, data); }
    uv_req_type get_type() const { __UVPP_CRTP_CAST_CONST(Derived); return uv_req_get_type(sub->ptr); }
    static const char* type_name(uv_req_type type) { return uv_req_type_name(type); }
};

template <typename Derived>
struct fs_request_mixin {
    uv_fs_type get_type() const { __UVPP_CRTP_CAST_CONST(Derived); return uv_fs_get_type(sub->ptr); }
    ssize_t get_result() const { __UVPP_CRTP_CAST_CONST(Derived); return uv_fs_get_result(sub->ptr); }
    int get_system_error() const { __UVPP_CRTP_CAST_CONST(Derived); return uv_fs_get_system_error(sub->ptr); }
    void* get_ptr() const { __UVPP_CRTP_CAST_CONST(Derived); return uv_fs_get_ptr(sub->ptr); }
    const char* get_path() const { __UVPP_CRTP_CAST_CONST(Derived); return uv_fs_get_path(sub->ptr); }
    uv_stat_t* get_statbuf() { __UVPP_CRTP_CAST(Derived); return uv_fs_get_statbuf(sub->ptr); }
};

} // namespace detail


#define UVPP_REQUEST_VIEW_DEF(cls) \
    void* ptr; \
    cls() : ptr(nullptr) {} \
    cls(void* ptr_) : ptr(ptr_) {}

#define UVPP_REQUEST_MANAGER_DEF(cls) \
    cls(const cls&) = delete; \
    cls(cls&& other) { ptr = other.ptr; other.ptr = nullptr; } \
    ~cls() { detail::dealloc_request(ptr); }


struct base_request_view :
    convertible_pointer_wrapper_mixin<base_request_view, uv_req_t>,
    detail::base_request_mixin<base_request_view>
{ UVPP_REQUEST_VIEW_DEF(base_request_view) };

struct base_request : base_request_view {
    base_request() : base_request_view(new uv_req_t()) { (*this)->type = UV_REQ; }
    UVPP_REQUEST_MANAGER_DEF(base_request)
};


namespace fs {

struct request_view :
    convertible_pointer_wrapper_mixin<request_view, uv_fs_t>,
    detail::base_request_mixin<request_view>,
    detail::fs_request_mixin<request_view>
{ UVPP_REQUEST_VIEW_DEF(request_view) };

struct request : request_view {
    request() : request_view(new uv_fs_t()) { (*this)->type = UV_FS; }
    UVPP_REQUEST_MANAGER_DEF(request)
};

} // namespace fs


} // namespace uvpp

# endif // __UVPP_STRUCT_WRAP_REQUESTS__
