//  :copyright: (c) 2017 Alex Huszagh.
//  :license: MIT, see licenses/mit.md for more details.
/**
 *  \addtogroup PyCPP
 *  \brief POSIX-specific path normalization routines.
 */

#include <pycpp/config.h>
#include <pycpp/preprocessor/os.h>

#if defined(OS_POSIX)                           // POSIX & MACOS
#   include <pycpp/filesystem.h>
#   include <pycpp/filesystem/exception.h>
#   include <pycpp/preprocessor/sysstat.h>
#   include <pycpp/stl/algorithm.h>
#   include <pycpp/string/unicode.h>
#   include <fcntl.h>
#   include <limits.h>
#   include <unistd.h>
#   include <wordexp.h>
#   include <assert.h>
#   include <stdlib.h>
#endif

PYCPP_BEGIN_NAMESPACE

#if defined(OS_POSIX)                           // POSIX & MACOS

// HELPERS
// -------


/**
 *  \brief Get iterator where last directory separator occurs.
 */
template <typename Path>
static typename Path::const_iterator stem_impl(const Path& path)
{
    using char_type = typename Path::value_type;

    return find_if(path.rbegin(), path.rend(), [](char_type c) {
        return path_separators.find(c) != path_separators.npos;
    }).base();
}


// RUNTIME

/**
 *  \brief Join POSIX-compliant paths to create path to full file.
 */
template <typename Path>
struct join_impl
{
    template <typename List, typename ToPath>
    Path operator()(const List &paths, ToPath topath)
    {
        Path path;
        for (auto &item: paths) {
            if (item.empty()) {
            } else if (path_separators.find(item[0]) != path_separators.npos) {
                path = Path(item);
            } else {
                path += Path(item);
            }
            path += topath(path_separator);
        }

        // clean up trailing separator
        if (path.size()) {
            path.erase(path.length() - 1);
        }

        return path;
    }
};

// SPLIT

template <typename Path>
static deque<Path> split_impl(const Path& path)
{
    auto it = stem_impl(path);
    Path basename(it, distance(it, path.cend()));
    Path dir(path.cbegin(), distance(path.cbegin(), it));
    if (dir.size() > 1 && path_separators.find(dir.back()) != path_separators.npos) {
        dir = dir.substr(0, dir.length() - 1);
    }

    return {dir, basename};
}


template <typename Path>
deque<Path> splitdrive_impl(const Path& path)
{
    return {Path(), path};
}


template <typename Path>
deque<Path> splitunc_impl(const Path& path)
{
    return {Path(), path};
}

// NORMALIZATION


template <typename Path>
static Path base_name_impl(const Path& path)
{
    auto it = stem_impl(path);
    return Path(it, distance(it, path.cend()));
}


template <typename Path>
static Path dir_name_impl(const Path& path)
{
    auto it = stem_impl(path);
    Path dir(path.cbegin(), distance(path.cbegin(), it));
    if (dir.size() > 1 && path_separators.find(dir.back()) != path_separators.npos) {
        dir = dir.substr(0, dir.length() - 1);
    }

    return dir;
}


template <typename Path>
struct expanduser_impl
{
    template <typename View>
    Path operator()(const View& path)
    {
        switch (path.size()) {
            case 0:
                return Path(path);
            case 1:
                return path[0] == '~' ? gethomedir() : Path(path);
            default: {
                if (path[0] == '~' && path_separators.find(path[1]) != path_separators.npos) {
                    return gethomedir() + Path(path.substr(1));
                }
                return Path(path);
            }
        }
    }
};


template <typename Path>
struct expandvars_impl
{
    template <typename View, typename FromPath, typename ToPath>
    Path operator()(const View& path, FromPath frompath, ToPath topath)
    {
        // handle any error codes
        wordexp_t word;
        switch (wordexp(frompath(path).data(), &word, 0)) {
            case 0:
                break;
            case WRDE_NOSPACE:
                // memory allocation issue, likely partially allocated
                wordfree(&word);
            default:
                return Path(path);
        }


        // process our words
        char** ptr = word.we_wordv;
        if (word.we_wordc == 0) {
            wordfree(&word);
            return Path(path);
        } else {
            auto out = topath(ptr[0]);
            wordfree(&word);
            return out;
        }
    }
};


template <typename Path>
bool isabs_impl(const Path& path)
{
    if (path.empty()) {
        return false;
    } else if (path_separators.find(path.front()) != path_separators.npos) {
        return true;
    }

    return false;
}

// MANIPULATION


/**
 *  \brief Use C FILE descriptors for high performance file copying.
 */
static bool copy_file_buffer(const path_view_t& src, const path_view_t& dst)
{
    assert(is_null_terminated(src));
    assert(is_null_terminated(dst));

    // choose a value between 4096 and 8 * 4096 (4-32KB).
    static constexpr size_t length = 2 * 4096;

    fd_t in = ::open(src.data(), O_RDONLY);
    if (in < 0) {
        return false;
    }
    fd_t out = ::open(dst.data(), O_WRONLY);
    if (out < 0) {
        ::close(in);
        return false;
    }

    char* buf = new char[length];
    int bytes = 0;
    while ((bytes = ::read(in, buf, length)) == length) {
        ::write(out, buf, length);
    }
    ::write(out, buf, bytes);

    delete[] buf;
    ::close(out);
    ::close(in);

    return true;
}


template <typename Path, typename MoveFile>
static bool move_file_impl(const Path& src, const Path& dst, bool replace, MoveFile move)
{
    auto dst_dir = dir_name(dst);

    // ensure we have a file and a dest directory
    auto src_stat = stat(src);
    auto dst_stat = stat(dst_dir);
    if (!isfile(src)) {
        throw filesystem_error(filesystem_not_a_file);
    } else if (!exists(dst_stat)) {
        throw filesystem_error(filesystem_no_such_directory);
    }

    // POSIX rename doesn't work accross filesystems
    // make sure stat data looks like the file was moved
    if (src_stat.st_dev != dst_stat.st_dev) {
        if (copy_file(src, dst, replace)) {
            return copystat(src, dst);
        }
        return false;
    }

    if (exists(dst)) {
        if (replace) {
            remove_file(dst);
        } else {
            throw filesystem_error(filesystem_destination_exists);
        }
    }

    return move(src, dst);
}


template <typename Path>
static bool is_empty_directory_impl(const Path& path)
{
    directory_iterator it(path);
    return it == directory_iterator();
}


template <typename Path>
static bool move_dir_impl(const Path& src, const Path& dst, bool replace)
{
    // get stats
    auto src_stat = stat(src);
    auto dst_stat = stat(dst);

    // check if we can move
    if (!isdir(src)) {
        throw filesystem_error(filesystem_no_such_directory);
    } else if (exists(dst_stat)) {
        // destination exists, can we overwrite?
        if (replace) {
            if (!remove_path(dst)) {
                return false;
            }
        } else if (!isdir(dst_stat) || !is_empty_directory_impl(dst)) {
            throw filesystem_error(filesystem_destination_exists);
        }
    }

    // check to see if they're on the same device
    if (src_stat.st_dev == dst_stat.st_dev) {
        // same filesystem, guaranteed atomicity
        return rename(src.data(), dst.data()) == 0;
    } else {
        // different filesystems, remove and copy
        if (!copy_dir(src, dst, true)) {
            throw filesystem_error(filesystem_unexpected_error);
        }
        if (!remove_dir(src)) {
            throw filesystem_error(filesystem_unexpected_error);
        }
    }

    return true;
}


template <typename Path, typename Mklink>
static bool mklink_impl(const Path& target, const Path& dst, bool replace, Mklink linker)
{
    if (replace && exists(dst)) {
        remove_file(dst);
    }

    return linker(target, dst);
}


template <typename Path, typename CopyFile>
static bool copy_file_impl(const Path& src, const Path& dst, bool replace, CopyFile copy)
{
    auto dst_dir = dir_name(dst);

    // ensure we have a file and a dest directory
    auto src_stat = stat(src);
    auto dst_stat = stat(dst_dir);
    if (!isfile(src)) {
        throw filesystem_error(filesystem_not_a_file);
    } else if (!exists(dst_stat)) {
        throw filesystem_error(filesystem_no_such_directory);
    }

    if (exists(dst)) {
        if (replace) {
            remove_file(dst);
        } else {
            throw filesystem_error(filesystem_destination_exists);
        }
    }

    return copy(src, dst);
}


template <typename Path>
static bool copy_dir_shallow_impl(const Path&src, const Path& dst)
{
    auto src_stat = stat(src);
    if (!exists(src_stat)) {
        throw filesystem_error(filesystem_no_such_directory);
    }

    return mkdir(dst, src_stat.st_mode);
}


template <typename Path>
static bool copy_dir_recursive_impl(const Path&src, const Path& dst)
{
    if (!copy_dir_shallow_impl(src, dst)) {
        return false;
    }

    directory_iterator first(src);
    directory_iterator last;
    for (; first != last; ++first) {
        path_t basename = first->basename();
        path_view_list_t dst_list = {dst, basename};
        if (first->isfile()) {
            if (!copy_file(first->path(), join_path(dst_list))) {
                return false;
            }
        } else if (first->islink()) {
            if (!copy_link(first->path(), join_path(dst_list))) {
                return false;
            }
        } else if (first->isdir()) {
            if (!copy_dir_recursive_impl(first->path(), join_path(dst_list))) {
                return false;
            }
        }
    }

    return true;
}


template <typename Path>
static bool copy_dir_impl(const Path&src, const Path& dst, bool recursive, bool replace)
{
    if (replace && exists(dst)) {
        if (!remove_path(dst)) {
            throw filesystem_error(filesystem_destination_exists);
        }
    }

    if (recursive) {
        return copy_dir_recursive_impl(src, dst);
    } else {
        return copy_dir_shallow_impl(src, dst);
    }
}

// FILE UTILS


static int convert_openmode(ios_base::openmode mode)
{
    int flags = 0;

    // read/write
    if ((mode & ios_base::in) && (mode & ios_base::out)) {
        flags |= O_RDWR;
    } else if (mode & ios_base::in) {
        flags |= O_RDONLY;
    } else if (mode & ios_base::out) {
        flags |= O_WRONLY;
    }

    // create file
    if (mode & ios_base::out) {
        flags |= O_CREAT;
    }

    // append
    if (mode & ios_base::app) {
        flags |= O_APPEND;
    }

    // truncate
    if (mode & ios_base::trunc) {
        flags |= O_TRUNC;
    }

    return flags;
}


template <typename Path>
static int fd_chmod_impl(const Path& path, mode_t permissions)
{
    fd_t fd = fd_open(path, ios_base::in | ios_base::out);
    if (fd < 0) {
        return false;
    }
    int status = fd_chmod(fd, permissions);
    fd_close(fd);       // ignore error, close() error makes no sense

    return status;
}


template <typename Path>
static int fd_allocate_impl(const Path& path, streamsize size)
{
    fd_t fd = fd_open(path, ios_base::out);
    if (fd < 0) {
        return false;
    }
    int status = fd_allocate(fd, size);
    fd_close(fd);       // ignore error, close() error makes no sense

    return status;
}


template <typename Path>
static int fd_truncate_impl(const Path& path, streamsize size)
{
    fd_t fd = fd_open(path, ios_base::out);
    if (fd < 0) {
        return false;
    }
    int status = fd_truncate(fd, size);
    fd_close(fd);       // ignore error, close() error makes no sense

    return status;
}

// POSIX FALLOCATE

#if defined(F_PREALLOCATE)                  // MACOS

/**
 *  \brief `posix_fallocate`-like implementation for macOS, which is missing it.
 */
static int fallocate_impl(int fd, off_t offset, off_t len)
{
    fstore_t store = {F_ALLOCATECONTIG, F_PEOFPOSMODE, offset, len, 0};
    int status = fcntl(fd, F_PREALLOCATE, &store);
    if (status == -1) {
        store.fst_flags = F_ALLOCATEALL;
        status = fcntl(fd, F_PREALLOCATE, &store);
    }

    if (status != -1) {
        // required for OS X to properly report the length
        // fnctl returns anything but -1 on success, but truncate returns
        // 0 on success, so we can guarantee -1 is error, 0 is success.
        status = ftruncate(fd, len);
    }

    return status;
}

#elif defined(F_ALLOCSP64)                  // F_ALLOCSP64

static int fallocate_impl(int fd, off_t offset, off_t len)
{
    // use F_ALLOCSP64 preferably, which allocates space
    // for the file if the OS supports it.
    flock64 fl;
    fl.l_whence = SEEK_SET;
    fl.l_start = offset;
    fl.l_len = len;

    return fcntl(fd, F_ALLOCSP64, &fl);
}

#elif defined(HAVE_POSIX_FALLOCATE)         // POSIX_FALLOCATE

static int fallocate_impl(int fd, off_t offset, off_t len)
{
    // `posix_fallocate` must allocate the space, even if the OS
    // does not support the behavior, leading the OS to write
    // 0s to all bytes in the file. Use only if absolutely
    // necessary.
    return posix_fallocate(fd, offset, len);
}

#else                                       // OTHER POSIX

static int fallocate_impl(int fd, off_t offset, off_t len)
{
    errno = EINVAL;
    return -1;
}

#endif                                      // MACOS

// POSIX FADVISE

#if defined(F_RDAHEAD)                      // MACOS

static int fadvise_impl(int fd, off_t offset, off_t len, io_access_pattern pattern)
{
    if (pattern == access_normal) {
        // no advise, skip
        return 0;
    }

    int arg;
    switch (pattern) {
        case access_sequential:
            arg = 1;                // enable read-ahead
            break;
        case access_random:
            arg = 0;                // disable read-ahead
            break;
        default:
            assert(false && "Unrecognized I/O access pattern.");
    }

    return fcntl(fd, F_RDAHEAD, arg);
}

#elif defined(HAVE_POSIX_FADVISE)           // POSIX_FADVISE

static int fadvise_impl(int fd, off_t offset, off_t len, io_access_pattern pattern)
{
    int advice;
    switch (pattern) {
        case access_normal:
            advice = POSIX_FADV_NORMAL;
            break;
        case access_sequential:
            advice = POSIX_FADV_SEQUENTIAL;
            break;
        case access_random:
            advice = POSIX_FADV_RANDOM;
            break;
        default:
            assert(false && "Unrecognized I/O access pattern.");
    }
    return posix_fadvise(fd, offset, len, advice);
}

#else                                       // OTHER POSIX

static int fadvise_impl(int fd, off_t offset, off_t len, io_access_pattern pattern)
{
    errno = EINVAL;
    return -1;
}

#endif                                      // MACOS

// CONSTANTS
// ---------

mode_t S_IWR_USR_GRP = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
mode_t S_IWRX_USR_GRP = S_IWR_USR_GRP | S_IXUSR | S_IXGRP;
mode_t S_IWR_USR_GRP_OTH = S_IWR_USR_GRP | S_IROTH | S_IWOTH;
mode_t S_IWRX_USR_GRP_OTH = S_IWRX_USR_GRP | S_IROTH | S_IWOTH | S_IXOTH;

// FUNCTIONS
// ---------

// RUNTIME

path_t getcwd()
{
    char* buf = new char[PATH_MAX];
    if (!::getcwd(buf, PATH_MAX)) {
        delete[] buf;
        throw filesystem_error(filesystem_unexpected_error);
    }

    path_t output(buf);
    delete[] buf;
    return output;
}


path_t join_path(const path_list_t &paths)
{
    return join_impl<path_t>()(paths, [](char c) {
        return c;
    });
}


path_t join_path(const path_view_list_t &paths)
{
    return join_impl<path_t>()(paths, [](char16_t c) {
        return c;
    });
}

// SPLIT

path_view_list_t path_split(const path_view_t& path)
{
    return split_impl(path);
}


path_view_list_t path_splitdrive(const path_view_t& path)
{
    return splitdrive_impl(path);
}


path_view_list_t path_splitunc(const path_view_t& path)
{
    return splitunc_impl(path);
}

// NORMALIZATION


bool isabs(const path_view_t& path)
{
    return isabs_impl(path);
}


path_view_t base_name(const path_view_t& path)
{
    return base_name_impl(path);
}


path_view_t dir_name(const path_view_t& path)
{
    return dir_name_impl(path);
}


path_t expanduser(const path_view_t& path)
{
    return expanduser_impl<path_t>()(path);
}


path_t expandvars(const path_view_t& path)
{
    auto frompath = [](const path_view_t& p) -> const path_view_t&
    {
        return p;
    };

    auto topath = [](const char* p) -> string
    {
        return path_t(p);
    };

    return expandvars_impl<path_t>()(path, frompath, topath);
}


path_t normcase(const path_view_t& path)
{
    return path_t(path);
}

// MANIPULATION


bool move_link(const path_view_t& src, const path_view_t& dst, bool replace)
{
    // POSIX rename works identically on files.
    return move_file(src, dst, replace);
}


bool move_file(const path_view_t& src, const path_view_t& dst, bool replace)
{
    assert(is_null_terminated(src));
    assert(is_null_terminated(dst));

    return move_file_impl(src, dst, replace, [](const path_view_t& src, const path_view_t& dst) {
        return rename(src.data(), dst.data()) == 0;
    });
}


bool move_dir(const path_view_t& src, const path_view_t& dst, bool replace)
{
    return move_dir_impl(src, dst, replace);
}


bool mklink(const path_view_t& target, const path_view_t& dst, bool replace)
{
    assert(is_null_terminated(target));
    assert(is_null_terminated(dst));

    return mklink_impl(target, dst, replace, [](const path_view_t& t, const path_view_t& d) {
        return symlink(d.data(), t.data()) == 0;
    });
}


bool copy_file(const path_view_t& src, const path_view_t& dst, bool replace)
{
    assert(is_null_terminated(src));
    assert(is_null_terminated(dst));

    return copy_file_impl(src, dst, replace, [](const path_view_t& src, const path_view_t& dst) {
        return copy_file_buffer(src, dst);
    });
}


bool remove_link(const path_view_t& path)
{
    assert(is_null_terminated(path));

    return remove_file(path);
}


bool remove_file(const path_view_t& path)
{
    assert(is_null_terminated(path));

    return unlink(path.data()) == 0;
}


static bool remove_dir_shallow_impl(const path_view_t& path)
{
    assert(is_null_terminated(path));

    return rmdir(path.data()) == 0;
}


static bool remove_dir_recursive_impl(const path_view_t& path)
{
    directory_iterator first(path);
    directory_iterator last;
    for (; first != last; ++first) {
        if (first->isfile()) {
            if (!remove_file(first->path())) {
                return false;
            }
        } else if (first->islink()) {
            if (!remove_link(first->path())) {
                return false;
            }
        } else if (first->isdir()) {
            if (!remove_dir_recursive_impl(first->path())) {
                return false;
            }
        }
    }

    return remove_dir_shallow_impl(path);
}


bool remove_dir(const path_view_t& path, bool recursive)
{
    if (recursive) {
        return remove_dir_recursive_impl(path);
    } else {
        return remove_dir_shallow_impl(path);
    }
}


bool copy_dir(const path_view_t& src, const path_view_t& dst, bool recursive, bool replace)
{
    assert(is_null_terminated(src));
    assert(is_null_terminated(dst));

    return copy_dir_impl(src, dst, recursive, replace);
}


bool mkdir(const path_view_t& path, int mode)
{
    assert(is_null_terminated(path));

    return ::mkdir(path.data(), static_cast<mode_t>(mode)) == 0;
}


bool makedirs(const path_view_t& path, int mode)
{
    if (!exists(path)) {
        makedirs(dir_name(path), mode);
        return mkdir(path, mode);
    }

    return false;
}

// FILE UTILS


fd_t fd_open(const path_view_t& path, ios_base::openmode openmode, mode_t permission, io_access_pattern access)
{
    assert(is_null_terminated(path));

    fd_t fd = ::open(path.data(), convert_openmode(openmode), permission);
    if (fd != INVALID_FD_VALUE) {
        if (fadvise_impl(fd, 0, 0, access) != 0) {
            // posix_fadvise not successful, close and return invalid handle
            fd_close(fd);
            fd = INVALID_FD_VALUE;
        }
    }

    return fd;
}


streamsize fd_read(fd_t fd, void* buf, streamsize count)
{
    return ::read(fd, buf, count);
}


streamsize fd_write(fd_t fd, const void* buf, streamsize count)
{
    return ::write(fd, buf, count);
}


streampos fd_seek(fd_t fd, streamoff off, ios_base::seekdir way)
{
    int whence;
    switch (way) {
        case ios_base::beg:
            whence = SEEK_SET;
            break;
        case ios_base::cur:
            whence = SEEK_CUR;
            break;
        case ios_base::end:
            whence = SEEK_END;
            break;
        default:
            return streampos(streamoff(-1));
    }

    return ::lseek(fd, off, whence);
}


int fd_close(fd_t fd)
{
    return ::close(fd);
}


int fd_chmod(fd_t fd, mode_t permissions)
{
    return ::fchmod(fd, permissions);
}


int fd_chmod(const path_view_t& path, mode_t permissions)
{
    return fd_chmod_impl(path, permissions);
}


int fd_allocate(fd_t fd, streamsize size)
{
    return fallocate_impl(fd, 0, size);
}


int fd_allocate(const path_view_t& path, streamsize size)
{
    return fd_allocate_impl(path, size);
}


int fd_truncate(fd_t fd, streamsize size)
{
    return ftruncate(fd, size);
}


int fd_truncate(const path_view_t& path, streamsize size)
{
    return fd_truncate_impl(path, size);
}

#endif

PYCPP_END_NAMESPACE
