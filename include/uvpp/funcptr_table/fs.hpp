#ifndef __UVPP_FUNCPTR_TABLE_FS__
#define __UVPP_FUNCPTR_TABLE_FS__

#include <unordered_map>

namespace uvpp::fs {

static std::unordered_map<void*, const char*> func_names = {
#include "uvpp/generated/funcptr_table/fs.hpp"
};

} // namespace uvpp::fs


#endif // __UVPP_FUNCPTR_TABLE_FS__
