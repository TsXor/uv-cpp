
UVPP_FN auto close(uv_loop_t* loop, uv_fs_t* req, uv_file file) {
    return coro_wrap(uv_fs_close, loop, req, file);
}

UVPP_FN auto open(uv_loop_t* loop, uv_fs_t* req, char const *path, int flags, int mode) {
    return coro_wrap(uv_fs_open, loop, req, path, flags, mode);
}

UVPP_FN auto read(uv_loop_t* loop, uv_fs_t* req, uv_file file, uv_buf_t const bufs[], unsigned int nbufs, int64_t offset) {
    return coro_wrap(uv_fs_read, loop, req, file, bufs, nbufs, offset);
}

UVPP_FN auto unlink(uv_loop_t* loop, uv_fs_t* req, char const *path) {
    return coro_wrap(uv_fs_unlink, loop, req, path);
}

UVPP_FN auto write(uv_loop_t* loop, uv_fs_t* req, uv_file file, uv_buf_t const bufs[], unsigned int nbufs, int64_t offset) {
    return coro_wrap(uv_fs_write, loop, req, file, bufs, nbufs, offset);
}

UVPP_FN auto copyfile(uv_loop_t* loop, uv_fs_t* req, char const *path, char const *new_path, int flags) {
    return coro_wrap(uv_fs_copyfile, loop, req, path, new_path, flags);
}

UVPP_FN auto mkdir(uv_loop_t* loop, uv_fs_t* req, char const *path, int mode) {
    return coro_wrap(uv_fs_mkdir, loop, req, path, mode);
}

UVPP_FN auto mkdtemp(uv_loop_t* loop, uv_fs_t* req, char const *tpl) {
    return coro_wrap(uv_fs_mkdtemp, loop, req, tpl);
}

UVPP_FN auto mkstemp(uv_loop_t* loop, uv_fs_t* req, char const *tpl) {
    return coro_wrap(uv_fs_mkstemp, loop, req, tpl);
}

UVPP_FN auto rmdir(uv_loop_t* loop, uv_fs_t* req, char const *path) {
    return coro_wrap(uv_fs_rmdir, loop, req, path);
}

UVPP_FN auto scandir(uv_loop_t* loop, uv_fs_t* req, char const *path, int flags) {
    return coro_wrap(uv_fs_scandir, loop, req, path, flags);
}

UVPP_FN auto opendir(uv_loop_t* loop, uv_fs_t* req, char const *path) {
    return coro_wrap(uv_fs_opendir, loop, req, path);
}

UVPP_FN auto readdir(uv_loop_t* loop, uv_fs_t* req, uv_dir_t *dir) {
    return coro_wrap(uv_fs_readdir, loop, req, dir);
}

UVPP_FN auto closedir(uv_loop_t* loop, uv_fs_t* req, uv_dir_t *dir) {
    return coro_wrap(uv_fs_closedir, loop, req, dir);
}

UVPP_FN auto stat(uv_loop_t* loop, uv_fs_t* req, char const *path) {
    return coro_wrap(uv_fs_stat, loop, req, path);
}

UVPP_FN auto fstat(uv_loop_t* loop, uv_fs_t* req, uv_file file) {
    return coro_wrap(uv_fs_fstat, loop, req, file);
}

UVPP_FN auto rename(uv_loop_t* loop, uv_fs_t* req, char const *path, char const *new_path) {
    return coro_wrap(uv_fs_rename, loop, req, path, new_path);
}

UVPP_FN auto fsync(uv_loop_t* loop, uv_fs_t* req, uv_file file) {
    return coro_wrap(uv_fs_fsync, loop, req, file);
}

UVPP_FN auto fdatasync(uv_loop_t* loop, uv_fs_t* req, uv_file file) {
    return coro_wrap(uv_fs_fdatasync, loop, req, file);
}

UVPP_FN auto ftruncate(uv_loop_t* loop, uv_fs_t* req, uv_file file, int64_t offset) {
    return coro_wrap(uv_fs_ftruncate, loop, req, file, offset);
}

UVPP_FN auto sendfile(uv_loop_t* loop, uv_fs_t* req, uv_file out_fd, uv_file in_fd, int64_t in_offset, size_t length) {
    return coro_wrap(uv_fs_sendfile, loop, req, out_fd, in_fd, in_offset, length);
}

UVPP_FN auto access(uv_loop_t* loop, uv_fs_t* req, char const *path, int mode) {
    return coro_wrap(uv_fs_access, loop, req, path, mode);
}

UVPP_FN auto chmod(uv_loop_t* loop, uv_fs_t* req, char const *path, int mode) {
    return coro_wrap(uv_fs_chmod, loop, req, path, mode);
}

UVPP_FN auto utime(uv_loop_t* loop, uv_fs_t* req, char const *path, double atime, double mtime) {
    return coro_wrap(uv_fs_utime, loop, req, path, atime, mtime);
}

UVPP_FN auto futime(uv_loop_t* loop, uv_fs_t* req, uv_file file, double atime, double mtime) {
    return coro_wrap(uv_fs_futime, loop, req, file, atime, mtime);
}

UVPP_FN auto lutime(uv_loop_t* loop, uv_fs_t* req, char const *path, double atime, double mtime) {
    return coro_wrap(uv_fs_lutime, loop, req, path, atime, mtime);
}

UVPP_FN auto lstat(uv_loop_t* loop, uv_fs_t* req, char const *path) {
    return coro_wrap(uv_fs_lstat, loop, req, path);
}

UVPP_FN auto link(uv_loop_t* loop, uv_fs_t* req, char const *path, char const *new_path) {
    return coro_wrap(uv_fs_link, loop, req, path, new_path);
}

UVPP_FN auto symlink(uv_loop_t* loop, uv_fs_t* req, char const *path, char const *new_path, int flags) {
    return coro_wrap(uv_fs_symlink, loop, req, path, new_path, flags);
}

UVPP_FN auto readlink(uv_loop_t* loop, uv_fs_t* req, char const *path) {
    return coro_wrap(uv_fs_readlink, loop, req, path);
}

UVPP_FN auto realpath(uv_loop_t* loop, uv_fs_t* req, char const *path) {
    return coro_wrap(uv_fs_realpath, loop, req, path);
}

UVPP_FN auto fchmod(uv_loop_t* loop, uv_fs_t* req, uv_file file, int mode) {
    return coro_wrap(uv_fs_fchmod, loop, req, file, mode);
}

UVPP_FN auto chown(uv_loop_t* loop, uv_fs_t* req, char const *path, uv_uid_t uid, uv_gid_t gid) {
    return coro_wrap(uv_fs_chown, loop, req, path, uid, gid);
}

UVPP_FN auto fchown(uv_loop_t* loop, uv_fs_t* req, uv_file file, uv_uid_t uid, uv_gid_t gid) {
    return coro_wrap(uv_fs_fchown, loop, req, file, uid, gid);
}

UVPP_FN auto lchown(uv_loop_t* loop, uv_fs_t* req, char const *path, uv_uid_t uid, uv_gid_t gid) {
    return coro_wrap(uv_fs_lchown, loop, req, path, uid, gid);
}

UVPP_FN auto statfs(uv_loop_t* loop, uv_fs_t* req, char const *path) {
    return coro_wrap(uv_fs_statfs, loop, req, path);
}
