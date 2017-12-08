//  :copyright: (c) 2017 Alex Huszagh.
//  :license: MIT, see licenses/mit.md for more details.
/**
 *  \addtogroup PyCPP
 *  \brief Decompressing stream definitions.
 */

#pragma once

#include <pycpp/compression/blosc.h>
#include <pycpp/compression/bzip2.h>
#include <pycpp/compression/detect.h>
#include <pycpp/compression/gzip.h>
#include <pycpp/compression/lzma.h>
#include <pycpp/compression/zlib.h>
#include <pycpp/stream/filter.h>

PYCPP_BEGIN_NAMESPACE

// MACROS
// ------

/**
 *  \brief Provides wide-path overloads for Windows.
 */
#if defined(HAVE_WFOPEN)                    // WINDOWS

#   define WIDE_PATH_IFSTREAM(name)                                                                             \
        name##_ifstream(const wstring_view& name, std::ios_base::openmode = std::ios_base::in);                 \
        void open(const wstring_view& name, std::ios_base::openmode = std::ios_base::in);                       \
        name##_ifstream(const u16string_view& name, std::ios_base::openmode = std::ios_base::in);               \
        void open(const u16string_view& name, std::ios_base::openmode = std::ios_base::in);

#   define WIDE_PATH_OFSTREAM(name)                                                                             \
        name##_ofstream(const wstring_view& name, std::ios_base::openmode = std::ios_base::out);                \
        name##_ofstream(const wstring_view& name, int level, std::ios_base::openmode = std::ios_base::out);     \
        void open(const wstring_view& name, std::ios_base::openmode = std::ios_base::out);                      \
        name##_ofstream(const u16string_view& name, std::ios_base::openmode = std::ios_base::out);              \
        name##_ofstream(const u16string_view& name, int level, std::ios_base::openmode = std::ios_base::out);   \
        void open(const u16string_view& name, std::ios_base::openmode = std::ios_base::out);

#else                                       // POSIX

#   define WIDE_PATH_IFSTREAM(name)
#   define WIDE_PATH_OFSTREAM(name)

#endif                                      // WINDOWS


/**
 *  \brief Macro to define a filtering istream base.
 */
#define COMPRESSED_ISTREAM(name)                                                                \
    struct name##_istream: filter_istream                                                       \
    {                                                                                           \
    public:                                                                                     \
        name##_istream();                                                                       \
        name##_istream(const name##_istream&) = delete;                                         \
        name##_istream & operator=(const name##_istream&) = delete;                             \
        ~name##_istream();                                                                      \
                                                                                                \
        name##_istream(std::istream& stream);                                                   \
        void open(std::istream& stream);                                                        \
                                                                                                \
    protected:                                                                                  \
        name##_istream(name##_istream&&);                                                       \
        name##_istream & operator=(name##_istream&&);                                           \
                                                                                                \
    private:                                                                                    \
        name##_decompressor ctx;                                                                \
    }


/**
 *  \brief Macro to define a filtering ostream base.
 */
#define COMPRESSED_OSTREAM(name)                                                                \
    struct name##_ostream: filter_ostream                                                       \
    {                                                                                           \
    public:                                                                                     \
        name##_ostream();                                                                       \
        name##_ostream(int level);                                                              \
        name##_ostream(const name##_ostream&) = delete;                                         \
        name##_ostream & operator=(const name##_ostream&) = delete;                             \
        ~name##_ostream();                                                                      \
                                                                                                \
        name##_ostream(std::ostream& stream);                                                   \
        name##_ostream(std::ostream& stream, int level);                                        \
        void open(std::ostream& stream);                                                        \
                                                                                                \
    protected:                                                                                  \
        name##_ostream(name##_ostream&&);                                                       \
        name##_ostream & operator=(name##_ostream&&);                                           \
                                                                                                \
    private:                                                                                    \
        name##_compressor ctx;                                                                  \
    }


/**
 *  \brief Macro to define a filtering ifstream base.
 */
#define COMPRESSED_IFSTREAM(name)                                                               \
    struct name##_ifstream: filter_ifstream                                                     \
    {                                                                                           \
    public:                                                                                     \
        name##_ifstream();                                                                      \
        name##_ifstream(const name##_ifstream&) = delete;                                       \
        name##_ifstream & operator=(const name##_ifstream&) = delete;                           \
        name##_ifstream(name##_ifstream&&);                                                     \
        name##_ifstream & operator=(name##_ifstream&&);                                         \
        ~name##_ifstream();                                                                     \
                                                                                                \
        name##_ifstream(const string_view& name, std::ios_base::openmode = std::ios_base::in);  \
        void open(const string_view& name, std::ios_base::openmode = std::ios_base::in);        \
        WIDE_PATH_IFSTREAM(name)                                                                \
                                                                                                \
    private:                                                                                    \
        name##_decompressor ctx;                                                                \
    }


/**
 *  \brief Macro to define a filtering ofstream base.
 */
#define COMPRESSED_OFSTREAM(name)                                                                           \
    struct name##_ofstream: filter_ofstream                                                                 \
    {                                                                                                       \
    public:                                                                                                 \
        name##_ofstream();                                                                                  \
        name##_ofstream(int level);                                                                         \
        name##_ofstream(const name##_ofstream&) = delete;                                                   \
        name##_ofstream & operator=(const name##_ofstream&) = delete;                                       \
        name##_ofstream(name##_ofstream&&);                                                                 \
        name##_ofstream & operator=(name##_ofstream&&);                                                     \
        ~name##_ofstream();                                                                                 \
                                                                                                            \
        name##_ofstream(const string_view& name, std::ios_base::openmode = std::ios_base::out);             \
        name##_ofstream(const string_view& name, int level, std::ios_base::openmode = std::ios_base::out);  \
        void open(const string_view& name, std::ios_base::openmode = std::ios_base::out);                   \
        WIDE_PATH_OFSTREAM(name)                                                                            \
                                                                                                            \
    private:                                                                                                \
        name##_compressor ctx;                                                                              \
    }


/**
 *  \brief Defines the compressed streams.
 */
#define COMPRESSED_STREAM_DEFINITION(name)                              \
    COMPRESSED_ISTREAM(name);                                           \
    COMPRESSED_OSTREAM(name);                                           \
    COMPRESSED_IFSTREAM(name);                                          \
    COMPRESSED_OFSTREAM(name)

// OBJECTS
// -------

#if defined(HAVE_BZIP2)
    COMPRESSED_STREAM_DEFINITION(bz2);
#endif

#if defined(HAVE_ZLIB)
    COMPRESSED_STREAM_DEFINITION(zlib);
    COMPRESSED_STREAM_DEFINITION(gzip);
#endif

#if defined(HAVE_LZMA)
    COMPRESSED_STREAM_DEFINITION(lzma);
#endif

/**
 *  \brief Compression-agnostic wrapper around an istream.
 */
struct decompressing_istream: filter_istream
{
public:
    decompressing_istream();
    decompressing_istream(const decompressing_istream&) = delete;
    decompressing_istream & operator=(const decompressing_istream&) = delete;
    ~decompressing_istream();

    decompressing_istream(std::istream& stream);
    void open(std::istream& stream);

protected:
    decompressing_istream(decompressing_istream&&);
    decompressing_istream & operator=(decompressing_istream&&);

private:
    compression_format format = compression_none;
    void *ctx = nullptr;
};


/**
 *  \brief Compression-agnostic wrapper around an ifstream.
 */
struct decompressing_ifstream: filter_ifstream
{
public:
    decompressing_ifstream();
    decompressing_ifstream(const decompressing_ifstream&) = delete;
    decompressing_ifstream & operator=(const decompressing_ifstream&) = delete;
    decompressing_ifstream(decompressing_ifstream&&);
    decompressing_ifstream & operator=(decompressing_ifstream&&);
    ~decompressing_ifstream();

    decompressing_ifstream(const string_view& name, std::ios_base::openmode = std::ios_base::in);
    void open(const string_view& name, std::ios_base::openmode = std::ios_base::in);
#if defined(HAVE_WFOPEN)                    // WINDOWS
    decompressing_ifstream(const wstring_view& name, std::ios_base::openmode = std::ios_base::in);
    void open(const wstring_view& name, std::ios_base::openmode = std::ios_base::in);
    decompressing_ifstream(const u16string_view& name, std::ios_base::openmode = std::ios_base::in);
    void open(const u16string_view& name, std::ios_base::openmode = std::ios_base::in);
#endif                                      // WINDOWS

private:
    compression_format format = compression_none;
    void *ctx = nullptr;
};

// CLEANUP
// -------

#undef WIDE_PATH_IFSTREAM
#undef WIDE_PATH_OFSTREAM
#undef COMPRESSED_ISTREAM
#undef COMPRESSED_OSTREAM
#undef COMPRESSED_IFSTREAM
#undef COMPRESSED_OFSTREAM
#undef COMPRESSED_STREAM_DEFINITION

PYCPP_END_NAMESPACE
