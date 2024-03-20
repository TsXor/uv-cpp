#pragma once
#ifndef __UVPP_MAKE_WINDOWS_HAPPY__
#define __UVPP_MAKE_WINDOWS_HAPPY__

#include <uv.h>
#include <vector>
#include <stdexcept>

namespace uvpp {

namespace detail {
static std::vector<char> argv_buf;    
} // namespace detail

// this functions calls Win32 API to make console accept utf-8
// and gets a stable utf-8 ARGV from wide-char (utf-16) ARGV 
// if _WIN32 is not defined, it will do nothing
static inline void make_windows_encoding_happy(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    
    int argcw; auto argvw = CommandLineToArgvW(GetCommandLineW(), &argcw);
    if (argvw == nullptr) { throw std::runtime_error("CommandLineToArgvW failure"); }
    if (argcw != argc) { throw std::runtime_error("argc mismatch"); }
    
    std::vector<int> argv_lens(argc);
    size_t argv_total_len = 0;
    for (int i = 0; i < argc; ++i) {
        int argi_size = WideCharToMultiByte(CP_UTF8, 0, argvw[i], -1, nullptr, 0, NULL, NULL);
        if (argi_size == 0) { throw std::runtime_error("WideCharToMultiByte failure"); }
        argv_lens[i] = argi_size;
        argv_total_len += argi_size;
    }
    detail::argv_buf.resize(argv_total_len);
    char* write_pos = detail::argv_buf.data();
    for (int i = 0; i < argc; ++i) {
        int write_size = WideCharToMultiByte(CP_UTF8, 0, argvw[i], -1, write_pos, argv_lens[i], NULL, NULL);
        if (write_size == 0) { throw std::runtime_error("WideCharToMultiByte failure"); }
        if (write_size != argv_lens[i]) { throw std::runtime_error("arg write length mismatch"); }
        argv[i] = write_pos;
        write_pos += write_size;
    }
#endif
}

} // namespace uvpp

#endif // __UVPP_MAKE_WINDOWS_HAPPY__
