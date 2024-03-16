#include <assert.h>
#include <stdio.h>
#include <fcntl.h>
#include <unordered_map>
#include <uvpp.hpp>


uvco::coro_task cat(uv_loop_t* loop, char* filename) try {
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
    // However, this has nothing to do with the result of filesystem
    // operation: an exception won't be thrown if a file is not found,
    // have no permission to read/write, etc. You should check if the
    // result field is negative.
    
    static const std::unordered_map<void*, const char*> fn_name_map = {
        {(void*)uv_fs_open, "uv_fs_open"},
        {(void*)uv_fs_read, "uv_fs_read"},
        {(void*)uv_fs_write, "uv_fs_write"}
    };
    auto fn_name_map_it = fn_name_map.find(err.func_ptr);
    const char* fn_name = fn_name_map_it == fn_name_map.end()
        ? "<unknown>" : fn_name_map_it->second;
    fprintf(stderr, "internal error in libuv function %s: %s\n", fn_name, err.str());
    co_return;
}

int main(int argc, char **argv) {
    auto loop = uvpp::default_loop();
    cat(loop, argv[1]);
    loop.run();
    return 0;
}
