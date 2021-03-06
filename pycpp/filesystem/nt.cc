//  :copyright: (c) 2017 Alex Huszagh.
//  :license: MIT, see licenses/mit.md for more details.
/**
 *  \addtogroup PyCPP
 *  \brief Windows-specific path normalization routines.
 */

#include <pycpp/config.h>
#include <pycpp/preprocessor/os.h>

#if defined(OS_WINDOWS)

#if _WIN32_WINNT < _WIN32_WINNT_VISTA
#   undef _WIN32_WINNT
#   define _WIN32_WINNT _WIN32_WINNT_VISTA
#endif

#include <pycpp/filesystem.h>
#include <pycpp/filesystem/exception.h>
#include <pycpp/preprocessor/errno.h>
#include <pycpp/preprocessor/sysstat.h>
#include <pycpp/stl/algorithm.h>
#include <pycpp/string/casemap.h>
#include <pycpp/string/unicode.h>
#include <pycpp/windows/error.h>
#include <pycpp/windows/winapi.h>
#include <warnings/push.h>
#include <warnings/narrowing-conversions.h>
#include <assert.h>
#include <io.h>

PYCPP_BEGIN_NAMESPACE

// HELPERS
// -------

// PATH

/**
 *  \brief Get iterator where last directory separator occurs.
 *  \warning splitdrive **must** be called prior to this.
 */
template <typename Path>
static typename Path::const_iterator stem_impl(const Path& path)
{
    using char_type = typename Path::value_type;

    return find_if(path.rbegin(), path.rend(), [](char_type c) {
        return path_separators.find(c) != path_separators.npos;
    }).base();
}


/**
 *  \brief Convert separators to preferred separators..
 */
template <typename Path>
struct make_preferred
{
    template <typename View>
    Path operator()(const View& path)
    {
        Path output;
        output.reserve(path.size());
        for (auto c: path) {
            if (path_separators.find(c) != path_separators.npos) {
                output.push_back(path_separator);
            } else {
                output.push_back(c);
            }
        }

        return output;
    }
};

// RUNTIME

/** Has support for multiple drives and UNC paths. A windows path is
 *  comprised of 2 parts: a drive, and a path from the root.
 *  Any absolute paths from the drive will replace previous roots,
 *  and any new drives will replace the root and the path.
 */
template <typename Path>
struct join_impl
{
    template <typename List, typename ToPath>
    Path operator()(const List &paths, ToPath topath)
    {
        Path drive, path;
        for (auto &item: paths) {
            auto split = path_splitdrive(item);
            if (split[0].size()) {
                // new drive
                drive = Path(split[0]);
                path = Path(split[1]);
                if (path.size()) {
                    // add only if non-empty, so join("D:", "temp") -> "D:temp"
                    path += topath(path_separator);
                }
            } else if (split[1].size()) {
                // skip empty elements
                auto &root = split[1];
                if (path_separators.find(root[0]) != path_separators.npos) {
                    // new root
                    path = Path(root);
                } else {
                    path += Path(root);
                }
                path += topath(path_separator);
            }
        }

        // clean up trailing separator
        if (path.size()) {
            path.erase(path.length() - 1);
        }

        return drive + path;
    }
};

// SPLIT

template <typename Path>
static deque<Path> split_impl(const Path& path)
{
    auto list = path_splitdrive(path);
    Path &tail = list.back();
    auto it = stem_impl(tail);
    Path basename(it, distance(it, tail.cend()));
    Path dir(tail.cbegin(), distance(tail.cbegin(), it));
    if (dir.size() > 1 && path_separators.find(dir.back()) != path_separators.npos) {
        dir = dir.substr(0, dir.length() - 1);
    }

    return {Path(path.begin(), distance(path.begin(), dir.end())), basename};
}


/**
 *  See [here](1) for information on Windows paths and labels.
 *  1. https://msdn.microsoft.com/en-us/library/windows/desktop/aa365247(v=vs.85).aspx
 *
 *  \code
 *      path_splitunc("\\\\localhost")  => {"", "\\\\localhost"}
 *      path_splitunc("\\\\localhost\\x")  => {"\\\\localhost\\x", ""}
 */
template <typename Path>
static deque<Path> splitunc_impl(const Path& path)
{
    // sanity checks
    if (path.size() < 2) {
        return {Path(), path};
    }

    if (path[1] == ':') {
        // have a drive letter
        return {Path(), path};
    }
    auto p0 = path_separators.find(path[0]) != path_separators.npos;
    auto p1 = path_separators.find(path[1]) != path_separators.npos;
    if (p0 && p1) {
        // have a UNC path
        auto norm = normcase(path);
        auto index = norm.find(path_separator, 2);
        if (index == norm.npos) {
            return {Path(), path};
        }
        index = norm.find(path_separator, index + 1);
        if (index == norm.npos) {
            return {path, Path()};
        }
        return {path.substr(0, index), path.substr(index)};
    }

    return {Path(), path};
}


/**
 *  See [here](1) for information on Windows paths and labels.
 *  1. https://msdn.microsoft.com/en-us/library/windows/desktop/aa365247(v=vs.85).aspx
 *
 *  \code
 *      path_splitdrive("\\\\localhost")  => {"", "\\\\localhost"}
 *      path_splitdrive("\\\\localhost\\x")  => {"\\\\localhost\\x", ""}
 *      path_splitdrive("\\\\localhost\\x\\y")  => {"\\\\localhost\\x", "\\y"}
 *      "\\\\?\\D:\\very long path" => {"\\\\?\\D:", "\\very long path"}
 */
template <typename Path>
static deque<Path> splitdrive_impl(const Path& path)
{
    if (path.size() < 2) {
        return {Path(), path};
    } else if (path[1] == ':') {
        return {path.substr(0, 2), path.substr(2)};
    }

    return splitunc_impl(path);
}

// NORMALIZATION


template <typename Path>
static Path base_name_impl(const Path& path)
{
    auto tail = path_splitdrive(path).back();
    auto it = stem_impl(tail);
    return Path(it, distance(it, tail.cend()));
}


template <typename Path>
static Path dir_name_impl(const Path& path)
{
    auto tail = path_splitdrive(path).back();
    auto it = stem_impl(tail);
    Path dir(tail.cbegin(), distance(tail.cbegin(), it));
    if (dir.size() > 1 && path_separators.find(dir.back()) != path_separators.npos) {
        dir = dir.substr(0, dir.length() - 1);
    }

    return dir;
}


template <typename Path>
struct expanduser_impl
{
    template <typename View, typename HomeFunc>
    Path operator()(const View& path, HomeFunc homefunc)
    {
        switch (path.size()) {
            case 0:
                return Path(path);
            case 1:
                return path[0] == '~' ? Path(homefunc()) : Path(path);
            default: {
                if (path[0] == '~' && path_separators.find(path[1]) != path_separators.npos) {
                    return Path(homefunc()) + Path(path.substr(1));
                }
                return Path(path);
            }
        }
    }
};


template <typename Path, typename Char>
struct expandvars_impl
{
    template <typename View, typename ToPath, typename Function>
    Path operator()(const View& path, ToPath topath, Function function)
    {
        Char* buf = new Char[MAX_PATH];
        auto data = reinterpret_cast<const Char*>(path.data());
        DWORD length = function(data, buf, MAX_PATH);
        if (length == 0) {
            delete[] buf;
            throw filesystem_error(filesystem_unexpected_error);
        }

        Path output(topath(buf, length-1));
        delete[] buf;
        return output;
    }
};


template <typename Path>
static bool isabs_impl(const Path& path)
{
    auto tail = splitdrive_impl(path).back();
    if (tail.empty()) {
        return false;
    } else if (path_separators.find(tail.front()) != path_separators.npos) {
        return true;
    }

    return false;
}


template <typename Path>
struct normcase_impl
{
    template <typename View, typename NormCase>
    Path operator()(const View& path, NormCase normcase)
    {
        return normcase(make_preferred<Path>()(path));
    }
};


// MANIPULATION

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

    DWORD flags = MOVEFILE_COPY_ALLOWED;
    if (exists(dst)) {
        if (replace) {
            flags |= MOVEFILE_REPLACE_EXISTING;
        } else {
            throw filesystem_error(filesystem_destination_exists);
        }
    }

    // Windows MoveFileW can handle different filesystems
    // don't worry about st_dev.
    return move(src, dst, flags);
}


template <typename Path>
static bool is_empty_directory_impl(const Path& path)
{
    directory_iterator it(path);
    return it == directory_iterator();
}


template <typename Path, typename MoveDir>
static bool move_dir_impl(const Path& src, const Path& dst, bool replace, MoveDir move)
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

    // no guaranteed atomicity
    if (src_stat.st_dev == dst_stat.st_dev) {
        // same volume, call MoveFile[AW]
        return move(src, dst);
    } else {
        // different filesystems, remove and copy
        if (!copy_dir(src, dst, true)) {
            throw filesystem_error(filesystem_unexpected_error);
        }
        if (!remove_dir(src)) {
            throw filesystem_error(filesystem_unexpected_error);
        }
    }
    return false;
}


template <typename Path, typename Mklink>
static bool mklink_impl(const Path& target, const Path& dst, bool replace, Mklink linker)
{
    if (replace && exists(dst)) {
        remove_file(dst);
    }

    DWORD flags = isdir(target);
    return linker(target, dst, flags);
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

    return copy(src, dst, replace);
}


static bool copy_dir_shallow_impl(const path_view_t&src, const path_view_t& dst)
{
    assert(is_null_terminated(src));
    assert(is_null_terminated(dst));

    auto s = reinterpret_cast<const wchar_t*>(src.data());
    auto d = reinterpret_cast<const wchar_t*>(dst.data());
    return CreateDirectoryExW(s, d, 0);
}


static bool copy_dir_shallow_impl(const backup_path_view_t&src, const backup_path_view_t& dst)
{
    assert(is_null_terminated(src));
    assert(is_null_terminated(dst));

    return CreateDirectoryExA(src.data(), dst.data(), 0);
}


static bool copy_dir_recursive_impl(const path_view_t& src, const path_view_t& dst)
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


static bool copy_dir_recursive_impl(const backup_path_view_t& src, const backup_path_view_t& dst)
{
    return copy_dir_recursive_impl(backup_path_to_path(src), backup_path_to_path(dst));
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


template <typename Path>
static bool remove_link_impl(const Path& path)
{
    if (!islink(path)) {
        throw filesystem_error(filesystem_not_a_symlink);
    }

    auto path_stat = stat(path);
    if (isdir(path_stat)) {
        return remove_dir(path, false);
    } else {
        return remove_file(path);
    }
}


static bool remove_dir_shallow_impl(const path_view_t& path)
{
    assert(is_null_terminated(path));

    auto p = reinterpret_cast<const wchar_t*>(path.data());
    return RemoveDirectoryW(p);
}


static bool remove_dir_shallow_impl(const backup_path_view_t& path)
{
    if (is_unicode(path)) {
        return remove_dir_shallow_impl(backup_path_to_path(path));
    }

    assert(is_null_terminated(path));

    return RemoveDirectoryA(path.data());
}


template <typename Path>
static bool remove_dir_recursive_impl(const Path& path)
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


template <typename Path>
static bool remove_dir_impl(const Path& path, bool recursive)
{
    if (recursive) {
        return remove_dir_recursive_impl(path);
    } else {
        return remove_dir_shallow_impl(path);
    }
}

// FILE UTILS


static DWORD convert_access_mode(ios_base::openmode mode)
{
    if ((mode & ios_base::in) && (mode & ios_base::out)) {
        return GENERIC_READ | GENERIC_WRITE;
    } else if (mode & ios_base::in) {
        return GENERIC_READ;
    } else if (mode & ios_base::out) {
        return GENERIC_WRITE;
    }

    return 0;
}


static DWORD convert_create_mode(ios_base::openmode mode)
{
    if (mode & ios_base::trunc) {
        return CREATE_ALWAYS;
    } else if (mode & ios_base::out) {
        return OPEN_ALWAYS;
    } else {
        return OPEN_EXISTING;
    }
}


static DWORD convert_acess_pattern(io_access_pattern pattern)
{
    switch (pattern) {
        case access_normal:
            return 0;
        case access_sequential:
            return FILE_FLAG_SEQUENTIAL_SCAN;
        case access_random:
            return FILE_FLAG_RANDOM_ACCESS;
        default:
            printf("Unrecognized I/O access pattern.");
            abort();
    }
}


template <typename Pointer, typename Function>
static HANDLE fd_open_impl(const Pointer &path, ios_base::openmode openmode, mode_t, io_access_pattern pattern, Function function)
{
    // ignore permissions since Windows uses a different
    // file-system permission model
    // Effectively, ignore it.
    DWORD access = convert_access_mode(openmode);
    DWORD share = 0;
    LPSECURITY_ATTRIBUTES security = nullptr;
    DWORD create = convert_create_mode(openmode);
    DWORD flags = convert_acess_pattern(pattern);
    HANDLE file = nullptr;

    return function(path, access, share, security, create, flags, file);
}


template <typename Path>
static int fd_chmod_impl(const Path& path, streamsize size)
{
    // Windows doesn't support POSIX-style permissions.
    // Null-op and return false.
    return -1;
}


template <typename Path>
static int fd_allocate_impl(const Path& path, streamsize size)
{
    fd_t fd = fd_open(path, ios_base::out);
    if (fd == INVALID_HANDLE_VALUE) {
        return false;
    }
    int status = fd_allocate(fd, size);
    fd_close(fd);

    return status;
}


template <typename Path>
static int fd_truncate_impl(const Path& path, streamsize size)
{
    fd_t fd = fd_open(path, ios_base::out);
    if (fd == INVALID_HANDLE_VALUE) {
        return false;
    }
    int status = fd_truncate(fd, size);
    fd_close(fd);

    return status;
}

// CONSTANTS
// ---------

// Windows has minimal permissions modes.
mode_t S_IWR_USR_GRP = _S_IREAD | _S_IWRITE;
mode_t S_IWRX_USR_GRP = S_IWR_USR_GRP;
mode_t S_IWR_USR_GRP_OTH = S_IWR_USR_GRP;
mode_t S_IWRX_USR_GRP_OTH = S_IWR_USR_GRP;

// FUNCTIONS
// ---------

// RUNTIME

path_t getcwd()
{
    wchar_t* buf = new wchar_t[MAX_PATH];
    DWORD length = GetCurrentDirectoryW(MAX_PATH, buf);
    if (length == 0) {
        delete[] buf;
        throw filesystem_error(filesystem_unexpected_error);
    }

    path_t output(reinterpret_cast<char16_t*>(buf), length);
    delete[] buf;
    return output;
}


path_t join_path(const path_list_t &paths)
{
    return join_impl<path_t>()(paths, [](char16_t c) {
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
    return expanduser_impl<path_t>()(path, gettempdirw);
}


path_t expandvars(const path_view_t& path)
{
    auto topath = [](wchar_t* str, size_t l) -> path_t
    {
        return path_t(reinterpret_cast<char16_t*>(str), l);
    };

    assert(is_null_terminated(path));

    return expandvars_impl<path_t, wchar_t>()(path, topath, ExpandEnvironmentStringsW);
}


path_t normcase(const path_view_t& path)
{
    return normcase_impl<path_t>()(path, [](const path_t& p) {
        return utf16_tolower(p);
    });
}

// MANIPULATION


bool move_link(const path_view_t& src, const path_view_t& dst, bool replace)
{
    // same as move_file
    return move_file(src, dst, replace);
}


bool move_file(const path_view_t& src, const path_view_t& dst, bool replace)
{
    assert(is_null_terminated(src));
    assert(is_null_terminated(dst));

    return move_file_impl(src, dst, replace, [](const path_view_t& src, const path_view_t& dst, DWORD f) {
        auto s = reinterpret_cast<const wchar_t*>(src.data());
        auto d = reinterpret_cast<const wchar_t*>(dst.data());
        return MoveFileExW(s, d, f);
    });
}


bool move_dir(const path_view_t& src, const path_view_t& dst, bool replace)
{
    assert(is_null_terminated(src));
    assert(is_null_terminated(dst));

    return move_dir_impl(src, dst, replace, [](const path_view_t& src, const path_view_t& dst) {
        auto s = reinterpret_cast<const wchar_t*>(src.data());
        auto d = reinterpret_cast<const wchar_t*>(dst.data());
        return MoveFileW(s, d);
    });
}


bool mklink(const path_view_t& target, const path_view_t& dst, bool replace)
{
    assert(is_null_terminated(target));
    assert(is_null_terminated(dst));

    return mklink_impl(target, dst, replace, [](const path_view_t& tar, const path_view_t& dst, DWORD f) {
        auto t = reinterpret_cast<const wchar_t*>(tar.data());
        auto d = reinterpret_cast<const wchar_t*>(dst.data());
        return CreateSymbolicLinkW(d, t, f);
    });
}


bool copy_file(const path_view_t& src, const path_view_t& dst, bool replace)
{
    assert(is_null_terminated(src));
    assert(is_null_terminated(dst));

    return copy_file_impl(src, dst, replace, [](const path_view_t& src, const path_view_t& dst, bool replace) {
        auto s = reinterpret_cast<const wchar_t*>(src.data());
        auto d = reinterpret_cast<const wchar_t*>(dst.data());
        return CopyFileW(s, d, replace);
    });
}


bool copy_dir(const path_view_t& src, const path_view_t& dst, bool recursive, bool replace)
{
    assert(is_null_terminated(src));
    assert(is_null_terminated(dst));

    return copy_dir_impl(src, dst, recursive, replace);
}


bool remove_link(const path_view_t& path)
{
    return remove_link_impl(path);
}


bool remove_file(const path_view_t& path)
{
    assert(is_null_terminated(path));

    return DeleteFileW(reinterpret_cast<const wchar_t*>(path.data()));
}


bool remove_dir(const path_view_t& path, bool recursive)
{
    return remove_dir_impl(path, recursive);
}


bool mkdir(const path_view_t& path, int mode)
{
    assert(is_null_terminated(path));

    auto data = reinterpret_cast<const wchar_t*>(path.data());
    if (CreateDirectoryW(data, nullptr)) {
        int mask = 0;
        // Windows doesn't allow write-only files. Keep for expressivity.
        if (mode & S_IRUSR) {
            mask |= _S_IREAD;
        }
        if (mode & S_IWUSR) {
            mask |= _S_IWRITE;
        }
        return _wchmod(data, mask) == 0;
    }

    return false;
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


fd_t fd_open(const path_view_t& path, ios_base::openmode mode, mode_t permission, io_access_pattern access)
{
    assert(is_null_terminated(path));

    const wchar_t* p = (const wchar_t*) path.data();
    fd_t fd = fd_open_impl(p, mode, permission, access, CreateFileW);
    if (fd == INVALID_HANDLE_VALUE) {
        set_errno_win32();
    }
    return fd;
}

streamsize fd_read(fd_t fd, void* buf, streamsize count)
{
    DWORD read;
    if (!ReadFile(fd, buf, count, &read, nullptr)) {
        set_errno_win32();
        return -1;
    }

    return read;
}


streamsize fd_write(fd_t fd, const void* buf, streamsize count)
{
    DWORD wrote;
    if (!WriteFile(fd, buf, count, &wrote, nullptr)) {
        set_errno_win32();
        return -1;
    }

    return wrote;
}


streampos fd_seek(fd_t fd, streamoff off, ios_base::seekdir way)
{
    DWORD method;
    switch (way) {
        case ios_base::beg:
            method = FILE_BEGIN;
            break;
        case ios_base::cur:
            method = FILE_CURRENT;
            break;
        case ios_base::end:
            method = FILE_END;
            break;
        default:
            return streampos(streamoff(-1));
    }

    LARGE_INTEGER in, out;
    in.QuadPart = off;
    if (!::SetFilePointerEx(fd, in, &out, method)) {
        set_errno_win32();
        return -1;          // force POSIX-like behavior
    }
    return out.QuadPart;
}


int fd_close(fd_t fd)
{
    if (!CloseHandle(fd)) {
        set_errno_win32();
        return -1;          // force POSIX-like behavior
    }
    return 0;
}


int fd_chmod(fd_t, mode_t)
{
    // Windows doesn't support Unix-style permissions.
    // All major cross-platform libraries ignore this.
    return false;
}


int fd_allocate(fd_t fd, streamsize size)
{
    if (fd == INVALID_HANDLE_VALUE) {
        return EBADF;
    }

    LARGE_INTEGER bytes;
    bytes.QuadPart = size;
    if (!::SetFilePointerEx(fd, bytes, nullptr, FILE_BEGIN)) {
        return translate_win32_error(GetLastError());
    }
    if (!::SetEndOfFile(fd)) {
        return translate_win32_error(GetLastError());
    }
    if (::SetFilePointer(fd, 0, nullptr, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
        return translate_win32_error(GetLastError());
    }

    return 0;
}


int fd_truncate(fd_t fd, streamsize size)
{
    return fd_allocate(fd, size);
}


int fd_chmod(const path_view_t& path, mode_t permissions)
{
    return fd_chmod_impl(path, permissions);
}


int fd_allocate(const path_view_t& path, streamsize size)
{
    return fd_allocate_impl(path, size);
}


int fd_truncate(const path_view_t& path, streamsize size)
{
    return fd_truncate_impl(path, size);
}

#endif


#if defined(OS_WINDOWS)         // BACKUP PATH

// RUNTIME

backup_path_t join_path(const backup_path_list_t &paths)
{
    return join_impl<backup_path_t>()(paths, [](char16_t c) {
        return static_cast<char>(c);
    });
}


backup_path_t join_path(const backup_path_view_list_t &paths)
{
    return join_impl<backup_path_t>()(paths, [](char16_t c) {
        return c;
    });
}

// STAT

bool isabs(const backup_path_view_t& path)
{
    return isabs_impl(path);
}

// SPLIT

backup_path_view_list_t path_split(const backup_path_view_t& path)
{
    return split_impl(path);
}


backup_path_view_list_t path_splitdrive(const backup_path_view_t& path)
{
    return splitdrive_impl(path);
}


backup_path_view_list_t path_splitunc(const backup_path_view_t& path)
{
    return splitunc_impl(path);
}

// NORMALIZATION

backup_path_view_t base_name(const backup_path_view_t& path)
{
    return base_name_impl(path);
}


backup_path_view_t dir_name(const backup_path_view_t& path)
{
    return dir_name_impl(path);
}


backup_path_t expanduser(const backup_path_view_t& path)
{
    if (is_unicode(path)) {
        return path_to_backup_path(expanduser(backup_path_to_path(path)));
    }

    return expanduser_impl<backup_path_t>()(path, gettempdira);
}


backup_path_t expandvars(const backup_path_view_t& path)
{
    if (is_unicode(path)) {
        return path_to_backup_path(expandvars(backup_path_to_path(path)));
    }

    auto topath = [](char* str, size_t l) -> backup_path_t
    {
        return backup_path_t(str, l);
    };

    assert(is_null_terminated(path));

    return expandvars_impl<backup_path_t, char>()(path, topath, ExpandEnvironmentStringsA);
}


backup_path_t normcase(const backup_path_view_t& path)
{
    return normcase_impl<backup_path_t>()(path, [](const backup_path_t& p) {
        return utf8_tolower(p);
    });
}

// MANIPULATION

bool move_link(const backup_path_view_t& src, const backup_path_view_t& dst, bool replace)
{
    // same as move_file
    return move_file(src, dst, replace);
}


bool move_file(const backup_path_view_t& src, const backup_path_view_t& dst, bool replace)
{
    if (is_unicode(src) || is_unicode(dst)) {
        return move_file(backup_path_to_path(src), backup_path_to_path(dst), replace);
    }

    assert(is_null_terminated(src));
    assert(is_null_terminated(dst));

    return move_file_impl(src, dst, replace, [](const backup_path_view_t& src, const backup_path_view_t& dst, DWORD f) {
        return MoveFileExA(src.data(), dst.data(), f);
    });
}


bool move_dir(const backup_path_view_t& src, const backup_path_view_t& dst, bool replace)
{
    if (is_unicode(src) || is_unicode(dst)) {
        return move_dir(backup_path_to_path(src), backup_path_to_path(dst), replace);
    }

    assert(is_null_terminated(src));
    assert(is_null_terminated(dst));

    return move_dir_impl(src, dst, replace, [](const backup_path_view_t& src, const backup_path_view_t& dst) {
        return MoveFileA(src.data(), dst.data());
    });
}


bool mklink(const backup_path_view_t& target, const backup_path_view_t& dst, bool replace)
{
    if (is_unicode(target) || is_unicode(dst)) {
        return mklink(backup_path_to_path(target), backup_path_to_path(dst), replace);
    }

    assert(is_null_terminated(target));
    assert(is_null_terminated(dst));

    return mklink_impl(target, dst, replace, [](const backup_path_view_t& t, const backup_path_view_t& d, DWORD f) {
        return CreateSymbolicLinkA(d.data(), t.data(), f);
    });
}


bool copy_file(const backup_path_view_t& src, const backup_path_view_t& dst, bool replace)
{
    if (is_unicode(src) || is_unicode(dst)) {
        return copy_file(backup_path_to_path(src), backup_path_to_path(dst), replace);
    }

    assert(is_null_terminated(src));
    assert(is_null_terminated(dst));

    return copy_file_impl(src, dst, replace, [](const backup_path_view_t& src, const backup_path_view_t& dst, bool replace) {
        return CopyFileA(src.data(), dst.data(), replace);
    });
}


bool copy_dir(const backup_path_view_t& src, const backup_path_view_t& dst, bool recursive, bool replace)
{
    assert(is_null_terminated(src));
    assert(is_null_terminated(dst));

    return copy_dir_impl(src, dst, recursive, replace);
}


bool remove_link(const backup_path_view_t& path)
{
    return remove_link_impl(path);
}


bool remove_file(const backup_path_view_t& path)
{
    if (is_unicode(path)) {
        return remove_file(backup_path_to_path(path));
    }

    assert(is_null_terminated(path));

    return DeleteFileA(path.data());
}


bool remove_dir(const backup_path_view_t& path, bool recursive)
{
    return remove_dir_impl(path, recursive);
}


bool mkdir(const backup_path_view_t& path, int mode)
{
    if (is_unicode(path)) {
        return mkdir(backup_path_to_path(path), mode);
    }

    assert(is_null_terminated(path));

    if (CreateDirectoryA(path.data(), nullptr)) {
        int mask = 0;
        if (mode & S_IRUSR) {
            mask |= _S_IREAD;
        }
        if (mode & S_IWUSR) {
            mask |= _S_IWRITE;
        }
        return _chmod(path.data(), mask) == 0;
    }

    return false;
}


bool makedirs(const backup_path_view_t& path, int mode)
{
    if (!exists(path)) {
        makedirs(dir_name(path), mode);
        return mkdir(path, mode);
    }

    return false;
}

// FILE UTILS

fd_t fd_open(const backup_path_view_t& path, ios_base::openmode mode, mode_t permission, io_access_pattern access)
{
    // Windows, Unicode API
    if (is_unicode(path)) {
        return fd_open(backup_path_to_path(path), mode, permission, access);
    }

    assert(is_null_terminated(path));

    fd_t fd = fd_open_impl(path.data(), mode, permission, access, CreateFileA);
    if (fd == INVALID_HANDLE_VALUE) {
        set_errno_win32();
    }
    return fd;
}


int fd_chmod(const backup_path_view_t& path, mode_t permissions)
{
    // Windows, Unicode API
    if (is_unicode(path)) {
        return fd_chmod(backup_path_to_path(path), permissions);
    }

    return fd_chmod_impl(path, permissions);
}


int fd_allocate(const backup_path_view_t& path, streamsize size)
{
    // Windows, Unicode API
    if (is_unicode(path)) {
        return fd_allocate(backup_path_to_path(path), size);
    }

    return fd_allocate_impl(path, size);
}


int fd_truncate(const backup_path_view_t& path, streamsize size)
{
    // Windows, Unicode API
    if (is_unicode(path)) {
        return fd_truncate(backup_path_to_path(path), size);
    }

    return fd_truncate_impl(path, size);
}

PYCPP_END_NAMESPACE

#include <warnings/pop.h>

#endif                              // WINDOWS
