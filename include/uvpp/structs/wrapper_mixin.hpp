#pragma once
#ifndef __UVPP_STRUCT_WRAP_MIXIN__
#define __UVPP_STRUCT_WRAP_MIXIN__

#include <uv.h>

#define __UVPP_CRTP_CAST(real_class) real_class* sub = static_cast<real_class*>(this)
#define __UVPP_CRTP_CAST_CONST(real_class) const real_class* sub = static_cast<const real_class*>(this)

namespace uvpp {

template <typename Derived, typename Wrapped>
struct convertible_pointer_wrapper_mixin {
    using raw_type = Wrapped*;
    using raw_const_type = const Wrapped*;
    raw_type operator->() { return *this; }
    raw_const_type operator->() const { return *this; }
    
    operator raw_type() {
        __UVPP_CRTP_CAST(Derived);
        return reinterpret_cast<raw_type>(sub->ptr);
    }
    operator raw_const_type() const {
        __UVPP_CRTP_CAST_CONST(Derived);
        return reinterpret_cast<raw_const_type>(sub->ptr);
    }
};

} // namespace uvpp

# endif // __UVPP_STRUCT_WRAP_MIXIN__
