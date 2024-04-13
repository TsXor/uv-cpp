#include <assert.h>
#include <stdio.h>
#include <fcntl.h>
#include <unordered_map>
#include <coutils.hpp>
#include <uvpp.hpp>
#include <uvpp/funcptr_table/fs.hpp>


coutils::async_fn<void> cat(uv_loop_t* loop, char* filename) try {
    uvpp::fs::request open_req, read_req, write_req, close_req;
    
    co_await uvco::fs::open(loop, open_req, filename, O_RDONLY, 0);
    if (open_req->result < 0) {
        fprintf(stderr, "error opening file: %s\n", uv_strerror(open_req->result));
        co_return;
    }
    
    char buffer[1024];
    uvpp::buf_view iov(buffer, sizeof(buffer));
    while (true) {
        co_await uvco::fs::read(loop, read_req, open_req->result, &iov, 1, -1);
        if (read_req->result < 0) {
            fprintf(stderr, "Read error: %s\n", uv_strerror(read_req->result));
            co_return;
        }
        
        if (read_req->result == 0) { break; }
        iov.len = read_req->result;
        
        co_await uvco::fs::write(loop, write_req, 1, &iov, 1, -1);
        if (write_req->result < 0) {
            fprintf(stderr, "Write error: %s\n", uv_strerror(write_req->result));
            co_return;
        }
    }
    
    co_await uvco::fs::close(loop, close_req, open_req->result);

} catch(const uvpp::api_error& err) {
    // For filesystem functions, a thrown exception means that some
    // "internal" fault happened: libuv loop refused to register our
    // coroutine to be continued later for some reason. (I think this
    // is an unrecoverable fault ;) )
    // Note: giving non-utf-8 parameter to libuv filesystem API is a
    // cause of such exceptions as well. On Windows, libuv converts
    // utf-8 parameter to utf-16 (Windows WCHAR encoding) before the
    // POST action. If parameter is not utf-8, converting will fail
    // before any operation is done.
    // However, this has nothing to do with the result of filesystem
    // operation: an exception won't be thrown if a file is not found,
    // have no permission to read/write, etc. You should check if the
    // result field is negative.
    
    const char* fn_name;
    try { fn_name = uvpp::fs::func_names.at(err.func_ptr); }
    catch(const std::out_of_range& err) { fn_name = "<unknown>"; }
    fprintf(stderr, "internal error in libuv function %s: %s\n", fn_name, err.str());
    co_return;
}

int main(int argc, char **argv) {
    uvpp::make_windows_encoding_happy(argc, argv);
    auto loop = uvpp::default_loop();
    coutils::sync::unleash(cat(loop, argv[1]));
    loop.run();
    return 0;
}
