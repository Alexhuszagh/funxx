//  :copyright: (c) 2017 Alex Huszagh.
//  :license: MIT, see licenses/mit.md for more details.
/**
 *  \addtogroup PyCPP
 *  \brief Private core module for encoding and decoding routines.
 */

#pragma once

#include <pycpp/compression/exception.h>
#include <pycpp/misc/safe_stdlib.h>
#include <pycpp/stl/type_traits.h>
#include <pycpp/stl/utility.h>
#include <pycpp/string/string.h>
#include <stdlib.h>

PYCPP_BEGIN_NAMESPACE

// MACROS
// ------

#define PYCPP_CHECK(EX) (void)((EX) >= 0 || (throw runtime_error(#EX), 0))

// CONSTANTS
// ---------

static constexpr int BUFFER_SIZE = 8092;

// DECLARATION
// -----------

/**
 *  \brief Implied base class for a compressor/decompressor.
 */
template <typename Stream>
struct filter_impl
{
    // MEMBER TYPES
    // ------------
    using next_in_type = remove_reference_t<decltype(declval<Stream>().next_in)>;
    using avail_in_type = remove_reference_t<decltype(declval<Stream>().avail_in)>;
    using next_out_type = remove_reference_t<decltype(declval<Stream>().next_out)>;
    using avail_out_type = remove_reference_t<decltype(declval<Stream>().avail_out)>;

    // MEMBER VARIABLES
    // ----------------
    int status;
    Stream stream;

    // MEMBER FUNCTIONS
    // ----------------
    filter_impl() noexcept;

    void before(void* dst, size_t dstlen) noexcept;
    void before(const void* src, size_t srclen, void* dst, size_t dstlen) noexcept;
    void after(void*& dst) noexcept;
    void after(const void*& src, void*& dst) noexcept;

    virtual void call() = 0;
    compression_status check_status(const void* src, void* dst, int stream_end) const noexcept;
    template <typename Cb> bool flush(void*& dst, size_t dstlen, Cb cb);
    compression_status operator()(const void*& src, size_t srclen, void*& dst, size_t dstlen, int stream_end);
};

// IMPLEMENTATION
// --------------

template <typename S>
filter_impl<S>::filter_impl() noexcept
{
    stream.avail_in = 0;
    stream.next_in = nullptr;
    stream.avail_out = 0;
    stream.next_out = nullptr;
}


template <typename S>
void filter_impl<S>::before(void* dst, size_t dstlen) noexcept
{
    stream.next_in = nullptr;
    stream.avail_in = 0;
    stream.next_out = (next_out_type) dst;
    stream.avail_out = (avail_out_type) dstlen;
}


template <typename S>
void filter_impl<S>::before(const void* src, size_t srclen, void* dst, size_t dstlen) noexcept
{
    // Use C-style cast, since we need to avoid spurious warnings
    // about integer size changes and bzip2 uses a non-const
    // input byte array.
    stream.next_in = (next_in_type) (src);
    stream.avail_in = (avail_in_type) srclen;
    stream.next_out = (next_out_type) dst;
    stream.avail_out = (avail_out_type) dstlen;
}


template <typename S>
void filter_impl<S>::after(void*& dst) noexcept
{
    dst = static_cast<void*>(stream.next_out);
}


template <typename S>
void filter_impl<S>::after(const void*& src, void*& dst) noexcept
{
    src = static_cast<const void*>(stream.next_in);
    dst = static_cast<void*>(stream.next_out);
}


template <typename S>
compression_status filter_impl<S>::check_status(const void* src, void* dst, int stream_end) const noexcept
{
    if (status == stream_end) {
        return compression_eof;
    } else if (static_cast<void*>(stream.next_out) == dst) {
        return compression_need_input;
    } else if (static_cast<const void*>(stream.next_in) == src) {
        return compression_need_output;
    }
    return compression_ok;
}


template <typename S>
template <typename Cb>
bool filter_impl<S>::flush(void*& dst, size_t dstlen, Cb cb)
{
    if (dst == nullptr) {
        return false;
    }
    before(dst, dstlen);
    bool code = cb();
    after(dst);

    return code;
}


template <typename S>
compression_status filter_impl<S>::operator()(const void*& src, size_t srclen, void*& dst, size_t dstlen, int stream_end)
{
    // no input data, or already reached stream end
    if (status == stream_end) {
        return compression_eof;
    } else if (srclen == 0 && stream.avail_in == 0) {
        return compression_need_input;
    } else if (dst == nullptr || dstlen == 0) {
        return compression_need_output;
    }

    bool use_src = (stream.next_in == nullptr || stream.avail_in == 0);
    if (use_src) {
        before(src, srclen, dst, dstlen);
    } else {
        // have remaining input data
        before(dst, dstlen);
    }

    call();

    compression_status code = check_status(src, dst, stream_end);
    if (use_src) {
        after(src, dst);
    } else {
        after(dst);
    }

    return code;
}

// FUNCTIONS
// ---------


template <typename Ctx>
string ctx_decompress(const string_wrapper& str)
{
    // configurations
    size_t dstlen = BUFFER_SIZE;
    size_t srclen = str.size();
    size_t dst_pos = 0;
    size_t src_pos = 0;
    char* buffer = (char*) safe_malloc(dstlen);
    void* dst = (void*) buffer;
    const void* src = (const void*) str.data();

    // initialize our decompression
    compression_status status = compression_ok;
    try {
        Ctx ctx;
        while (status != compression_eof) {
            dstlen *= 2;
            buffer = (char*) safe_realloc(buffer, dstlen);
            dst = (void*) (buffer + dst_pos);
            status = ctx.decompress(src, srclen - src_pos, dst, dstlen - dst_pos);
            dst_pos = distance(buffer, (char*) dst);
            src_pos = distance(str.data(), (const char*) src);
        }

        // flush remaining buffer
        ctx.flush(dst, dstlen - dst_pos);
        dst_pos = distance(buffer, (char*) dst);

    } catch (...) {
        safe_free(dst);
        throw;
    }

    // create our output string
    size_t out = distance(buffer, (char*) dst);
    string output(buffer, out);
    safe_free(buffer);

    return output;
}


template <typename Function>
string compress_bound(const string_wrapper& str, size_t dstlen, Function function)
{
    const char* src = str.data();
    char *dst = (char*) safe_malloc(dstlen);
    const void* src_first = (const void*) src;
    void* dst_first = (void*) dst;

    try {
        function(src_first, str.size(), dst_first, dstlen);
    } catch (...) {
        safe_free(dst);
        throw;
    }
    size_t length = distance(dst, (char*) dst_first);
    string output(dst, length);
    safe_free(dst);

    return output;
}


template <typename Function>
string decompress_bound(const string_wrapper& str, size_t bound, Function function)
{
    const char* src = str.data();
    char *dst = (char*) safe_malloc(bound);
    const void* src_first = (const void*) src;
    void* dst_first = (void*) dst;

    try {
        function(src_first, str.size(), dst_first, bound, bound);
    } catch (...) {
        safe_free(dst);
        throw;
    }
    size_t length = distance(dst, (char*) dst_first);
    string output(dst, length);
    safe_free(dst);

    return output;
}


PYCPP_END_NAMESPACE
