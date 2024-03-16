#pragma once
#ifndef __UVPP_STRUCT_WRAP_MISC__
#define __UVPP_STRUCT_WRAP_MISC__

#include <uv.h>

namespace uvpp {

// this is small enough
struct buf_view : uv_buf_t {
    buf_view(char* base, unsigned int len) { *(uv_buf_t*)this = uv_buf_init(base, len); }
};

} // namespace uvpp

# endif // __UVPP_STRUCT_WRAP_MISC__
