#pragma once
#ifndef __UVPP_ERROR__
#define __UVPP_ERROR__

#include <uv.h>

namespace uvpp {

struct api_error {
    void* func_ptr; // a pointer to raw libuv API function who caused this error
    int errcode; // error code
    template <typename FuncT>
    api_error(FuncT* func_ptr_, int errcode_):
        func_ptr(reinterpret_cast<void*>(func_ptr_)), errcode(errcode) {}
    const char* str() const { return uv_strerror(errcode); }
};

} // namespace uvpp

# endif // __UVPP_ERROR__
