//  :copyright: (c) 2017 Alex Huszagh.
//  :license: MIT, see licenses/mit.md for more details.
/**
 *  \addtogroup PyCPP
 *  \brief POSIX-specific path normalization routines.
 */

#include <pycpp/config.h>
#include <pycpp/os.h>

#if defined(OS_POSIX)                           // POSIX & MACOS
#include <pycpp/filesystem.h>
#include <pycpp/filesystem/exception.h>
#include <pycpp/unicode.h>
#include <limits.h>
#include <unistd.h>
#include <wordexp.h>
#include <algorithm>
#include <cstdlib>
#endif

PYCPP_BEGIN_NAMESPACE

#if defined(OS_POSIX)                           // POSIX & MACOS

// HELPERS
// -------


/**
 *  \brief Get path to home directory.
 */
template <typename Path, typename ToPath>
static Path home_impl(ToPath topath)
{
    char *dir = std::getenv("HOME");
    if (dir != nullptr) {
        return topath(dir);
    } else {
        // home not defined, root user
        return topath("/");
    }
}


/**
 *  \brief Get path to temporary directory.
 */
template <typename Path, typename ToPath>
static Path tmpdir_impl(ToPath topath)
{
    char *dir = std::getenv("TMPDIR");
    if (dir != nullptr) {
        return topath(dir);
    }

    dir = std::getenv("TEMP");
    if (dir != nullptr) {
        return topath(dir);
    }

    dir = std::getenv("TMP");
    if (dir != nullptr) {
        return topath(dir);
    } else {
        // temp directory not defined, return root
        return topath("/");
    }
}


/**
 *  \brief Get iterator where last directory separator occurs.
 */
template <typename Path>
static typename Path::const_iterator stem_impl(const Path& path)
{
    typedef typename Path::value_type char_type;

    return std::find_if(path.rbegin(), path.rend(), [](char_type c) {
        return path_separators.find(c) != path_separators.npos;
    }).base();
}


// RUNTIME

/** \brief Join POSIX-compliant paths to create path to full file.
 */
template <typename List, typename ToPath>
static typename List::value_type join_impl(const List &paths, ToPath topath)
{
    typename List::value_type path;
    for (auto &item: paths) {
        if (item.empty()) {
        } else if (path_separators.find(item[0]) != path_separators.npos) {
            path = item;
        } else {
            path += item;
        }
        path += topath(path_separator);
    }

    // clean up trailing separator
    if (path.size()) {
        path.erase(path.length() - 1);
    }

    return path;
}

// SPLIT

template <typename Path>
static std::deque<Path> split_impl(const Path& path)
{
    auto it = stem_impl(path);
    Path basename(it, path.cend());
    Path dir(path.cbegin(), it);
    if (dir.size() > 1 && path_separators.find(dir.back()) != path_separators.npos) {
        dir.erase(dir.length() - 1);
    }

    return {std::move(dir), std::move(basename)};
}


template <typename Path>
std::deque<Path> splitdrive_impl(const Path& path)
{
    return {Path(), path};
}


template <typename Path>
std::deque<Path> splitunc_impl(const Path& path)
{
    return {Path(), path};
}

// NORMALIZATION


template <typename Path>
static Path base_name_impl(const Path& path)
{
    return Path(stem_impl(path), path.cend());
}


template <typename Path>
static Path dir_name_impl(const Path& path)
{
    auto it = stem_impl(path);
    Path dir(path.cbegin(), it);
    if (dir.size() > 1 && path_separators.find(dir.back()) != path_separators.npos) {
        dir.erase(dir.length() - 1);
    }

    return dir;
}


template <typename Path, typename ToPath>
static Path expanduser_impl(const Path& path, ToPath topath)
{
    switch (path.size()) {
        case 0:
            return path;
        case 1:
            return path[0] == '~' ? home_impl<Path>(topath) : path;
        default: {
            if (path[0] == '~' && path_separators.find(path[1]) != path_separators.npos) {
                return home_impl<Path>(topath) + path.substr(1);
            }
            return path;
        }
    }
}


template <typename Path, typename FromPath, typename ToPath>
static Path expandvars_impl(const Path& path, FromPath frompath, ToPath topath)
{
    wordexp_t word;
    wordexp(frompath(path).data(), &word, 0);
    char** ptr = word.we_wordv;
    if (word.we_wordc == 0) {
        wordfree(&word);
        return path;
    } else {
        auto out = topath(ptr[0]);
        wordfree(&word);
        return out;
    }
}


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
static bool copy_file_buffer(const path_t& src, const path_t& dst)
{
    static constexpr size_t length = 4096;

    FILE *in = fopen(src.data(), "rb");
    if (!in) {
        return false;
    }
    FILE *out = fopen(dst.data(), "wb");
    if (!out) {
        return false;
    }

    char* buf = new char[length];
    int read = 0;
    while ((read = fread(buf, 1, length, in)) == length) {
        fwrite(buf, 1, length, out);
    }
    fwrite(buf, 1, read, out);

    delete[] buf;
    fclose(out);
    fclose(in);

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
        return copy_file(src, dst, replace, true);
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


template <typename Path, typename Mklink>
static bool mklink_impl(const Path& target, const Path& dst, bool replace, Mklink linker)
{
    if (replace && exists(dst)) {
        remove_file(dst);
    }

    return linker(target, dst);
}


template <typename Path, typename CopyFile>
static bool copy_file_impl(const Path& src, const Path& dst, bool replace, bool copystat, CopyFile copy)
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

    bool status = copy(src, dst);
    if (status && copystat) {
        return PYCPP_NAMESPACE::copystat(src, dst);
    }
    return status;
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
        path_list_t dst_list = {dst, first->basename()};
        if (first->isfile()) {
            if (!copy_file(first->path(), join(dst_list))) {
                return false;
            }
        } else if (first->islink()) {
            if (!copy_link(first->path(), join(dst_list))) {
                return false;
            }
        } else if (first->isdir()) {
            if (!copy_dir_recursive_impl(first->path(), join(dst_list))) {
                return false;
            }
        }
    }

    return true;
}


template <typename Path>
static bool copy_dir_impl(const Path&src, const Path& dst, bool recursive)
{
    if (recursive) {
        return copy_dir_recursive_impl(src, dst);
    } else {
        return copy_dir_shallow_impl(src, dst);
    }
}


// FUNCTIONS
// ---------

// RUNTIME

path_t getcwd()
{
    char* buf = new char[PATH_MAX];
    if (!::getcwd(buf, PATH_MAX)) {
        throw filesystem_error(filesystem_unexpected_error);
    }

    path_t output(buf);
    delete[] buf;
    return output;
}


path_t join(const path_list_t &paths)
{
    return join_impl(paths, [](char c) {
        return c;
    });
}

// SPLIT

path_list_t split(const path_t& path)
{
    return split_impl(path);
}


path_list_t splitdrive(const path_t& path)
{
    return splitdrive_impl(path);
}


path_list_t splitunc(const path_t& path)
{
    return splitunc_impl(path);
}

// NORMALIZATION


bool isabs(const path_t& path)
{
    return isabs_impl(path);
}


path_t base_name(const path_t& path)
{
    return base_name_impl(path);
}


path_t dir_name(const path_t& path)
{
    return dir_name_impl(path);
}


path_t expanduser(const path_t& path)
{
    return expanduser_impl(path, [](const path_t& p) {
        return p;
    });
}


path_t expandvars(const path_t& path)
{
    auto frompath = [](const path_t& p) {
        return p;
    };
    auto topath = [](const char* p) {
        return p;
    };

    return expandvars_impl(path, frompath, topath);
}


path_t normcase(const path_t& path)
{
    return path;
}

// MANIPULATION

bool move_file(const path_t& src, const path_t& dst, bool replace)
{
    return move_file_impl(src, dst, replace, [](const path_t& src, const path_t& dst) {
        return rename(src.data(), dst.data()) == 0;
    });
}


bool mklink(const path_t& target, const path_t& dst, bool replace)
{
    return mklink_impl(target, dst, replace, [](const path_t& t, const path_t& d) {
        return symlink(d.data(), t.data()) == 0;
    });
}


bool copy_file(const path_t& src, const path_t& dst, bool replace, bool copystat)
{
    return copy_file_impl(src, dst, replace, copystat, [](const path_t& src, const path_t& dst) {
        return copy_file_buffer(src, dst);
    });
}


bool remove_file(const path_t& path)
{
    return unlink(path.data()) == 0;
}


bool copy_dir(const path_t& src, const path_t& dst, bool recursive)
{
    return copy_dir_impl(src, dst, recursive);
}


bool mkdir(const path_t& path, int mode)
{
    return PYCPP_NAMESPACE::mkdir(path.data(), mode) == 0;
}


bool makedirs(const path_t& path, int mode)
{
    if (!exists(path)) {
        makedirs(dir_name(path), mode);
        return mkdir(path, mode);
    }

    return false;
}

#endif

PYCPP_END_NAMESPACE
