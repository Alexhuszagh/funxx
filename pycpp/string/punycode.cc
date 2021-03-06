//  :copyright: (c) 2011 by Ben Noordhuis <info@bnoordhuis.nl>.
//  :copyright: (c) 2017 Alex Huszagh.
//  :license: Unicode, see licenses/mit.md for more details.

#include <pycpp/misc/safe_stdlib.h>
#include <pycpp/stl/algorithm.h>
#include <pycpp/stl/limits.h>
#include <pycpp/stl/stdexcept.h>
#include <pycpp/string/punycode.h>
#include <pycpp/string/unicode.h>
#include <math.h>
#include <string.h>

PYCPP_BEGIN_NAMESPACE

// HELPERS
// -------


/**
 *  \brief Bias adaption function.
 */
uint32_t adapt_bias(uint32_t delta, uint32_t points, bool first) noexcept
{
    // variables
    static constexpr uint32_t base = 36;
    static constexpr uint32_t damp = 700;
    static constexpr uint32_t skew = 38;
    static constexpr uint32_t tmin = 1;
    static constexpr uint32_t tmax = 26;

    // initialize
    delta /= first ? damp : 2;
    delta += delta / points;

    /* while delta > 455: delta /= 35 */
    uint32_t k;
    for (k = 0; delta > ((base - tmin) * tmax) / 2; k += base) {
        delta /= (base - tmin);
    }

    return k + (((base - tmin + 1) * delta) / (delta + skew));
}

/**
 *  \brief Get threshold, t(j) from RFC3492.
 */
uint32_t threshold(uint32_t k, uint32_t bias) noexcept
{
    // limits
    static constexpr uint32_t tmin = 1;
    static constexpr uint32_t tmax = 26;

    if (k <= bias) {
        return tmin;
    } else if (k >= bias + tmax) {
        return tmax;
    } else {
        return k - bias;
    }
}


/**
 *  \brief Encode single byte to Punycode.
 */
static uint8_t encode_byte(uint8_t c)
{
    // limits
    static constexpr uint32_t base = 36;
    static constexpr uint32_t tmin = 1;
    static constexpr uint32_t range = base - tmin;

    if (c < 0 || c > range) {
        throw out_of_range("Character to encode is out of range\n");
    }
    if (c > 25) {
        return c + 22;                      // '0'..'9'
    }
    return c + 'a';                         // 'a'..'z'
}


/**
 *  \brief Decode single byte from Punycode.
 */
static uint32_t decode_byte(uint8_t c) noexcept
{
    // limits
    static constexpr uint32_t base = 36;

    if ((c >= 48) && (c <= 57)) {
        return c - 22;                      // '0'..'9'
    } else if ((c >= 97) && (c <= 122)) {
        return c - 'a';                     // 'a'..'z'
    } else if ((c >= 65) && (c <= 90)) {
        return c - 'A';                     // 'A'..'Z'
    }

    return base;
}


/**
 *  \brief Encode single UTF32 code point to Punycode.
 */
template <typename Iter8>
size_t encode_character(uint32_t bias, uint32_t delta, Iter8 &first, Iter8 last)
{
    // limits
    static constexpr uint32_t base = 36;

    // encode
    uint32_t t;
    uint32_t k = base;
    uint32_t q = delta;
    size_t bytes = distance(first, last);

    while (first < last) {
        t = threshold(k, bias);

        if (q < t) {
            break;
        }

        *first++ = encode_byte(t + (q - t) % (base - t));
        q = (q - t) / (base - t);
        k += base;
    }

    if (first < last) {
        *first++ = encode_byte(q);
    }

    return bytes;
}


/**
 *  \brief Encode UTF32 array to Punycode.
 */
template <typename Iter32, typename Iter8>
void encode_impl(Iter32 &src_first, Iter32 src_last,
                 Iter8 &dst_first, Iter8 dst_last)
{
    // limits
    static constexpr uint32_t ascii_end = 128;
    Iter32 src = src_first;
    Iter8 dst = dst_first;

    // encode basic
    while (src < src_last) {
        uint32_t c = *src++;
        if (c < ascii_end) {
            *dst++ = c;
        }
    }

    // get number processed
    uint32_t basic = static_cast<uint32_t>(dst - dst_first);
    uint32_t h = basic;
    if (basic) {
        *dst++ = '-';
    }

    // process non-basic characters
    uint32_t n = ascii_end;
    uint32_t bias = 72;
    uint32_t delta = 0;
    size_t srclen = src_last - src_first;
    size_t m;

    for (; h < srclen && dst < dst_last; n++, delta++) {
        // find smallest non-basic code point
        src = src_first;
        for (m = SIZE_MAX; src < src_last; ++src) {
            uint32_t c = *src;
            if (c >= n && c < m) {
                m = c;
            }
        }

        // check overflow
        if ((m - n) > (SIZE_MAX - delta) / (h + 1)) {
            throw overflow_error("Overflow detected in encoding.\n");
        }

        delta += static_cast<uint32_t>((m - n) * (h + 1));
        n = static_cast<uint32_t>(m);

        // add values
        for (src = src_first; src < src_last; ++src) {
            uint32_t c = *src;
            if (c < n) {
                if (++delta == 0) {
                    throw overflow_error("Punycode encoding overflow.\n");
                }
            } else if (c == n) {
                encode_character(bias, delta, dst, dst_last);
                bias = adapt_bias(delta, h + 1, h == basic);
                delta = 0;
                ++h;
            }
        }
    }

    // reset pointers
    src_first = src;
    dst_first = dst;
}


/**
 *  \brief Encode Punycode array to UTF32.
 */
template <typename Iter8, typename Iter32>
void decode_impl(Iter8 &src, size_t srclen,
                 Iter32 &dst, size_t dstlen)
{
    static constexpr uint32_t int32_max = numeric_limits<int32_t>::max();
    static constexpr uint32_t base = 36;
    static constexpr size_t initial_n = 128;
    static constexpr size_t initial_bias = 72;
    static constexpr size_t tmin = 1;
    static constexpr size_t tmax = 26;

    // parameters
    size_t i = 0;
    size_t n = initial_n;
    size_t bias = initial_bias;

    // check for unicode characters
    if (any_of(src, src+srclen, [](char c) {
        return !is_ascii(c);
    })) {
        return;
    }

    // reverse search for delimiter
    Iter8 ptr;
    for (ptr = src+srclen-1; ptr > src && *ptr != '-'; --ptr);
    size_t basic = ptr - src;
    size_t si = 0;
    size_t di = min(basic, dstlen);
    copy(src, src+di, dst);

    for (si = basic + (basic > 0); si < srclen && di < dstlen; di++) {
        size_t oldi = i;
        for (size_t w = 1, k = base; di < dstlen; k += base) {
            auto digit = decode_byte(src[si++]);
            if (digit >= base || digit > (int32_max - i) / w) {
                throw overflow_error("Overflow in Punycode decode.");
            }

            i += digit * w;
            size_t t;
            if (k <= bias) {
                t = tmin;
            } else if (k >= bias + tmax) {
                t = tmax;
            } else {
                t = k - bias;
            }

            if (digit < t) {
                break;
            }

            if (w > INT32_MAX / (base - t)) {
                throw overflow_error("Overflow in Punycode decode.");
            }
            w *= (base - t);
        }

        size_t x = di + 1;
        bias = adapt_bias(static_cast<uint32_t>(i - oldi), static_cast<uint32_t>(x), oldi == 0);
        if (i / x > INT32_MAX - n) {
            throw overflow_error("Overflow in Punycode decode.");
        }

        n += i / x;
        i %= x;
        memmove(dst + i + 1, dst + i, (di - i) * 4);
        dst[i++] = static_cast<uint32_t>(n);
    }

    src += si;
    dst += di;
}


template <typename Cb1, typename Cb2>
static void punycode_conversion(const void*& src,
    size_t srclen,
    void*& dst,
    size_t dstlen,
    const byte_allocator& allocator,
    Cb1 cb1,
    Cb2 cb2)
{
    // get allocator
    using char_allocator = allocator_traits<byte_allocator>::template rebind_alloc<char>;
    char_allocator alloc(allocator);

    // get preferred formats
    size_t u32_size = srclen * 4;
    char* u32 = nullptr;

    // convert
    try {
        // allocate memory
        u32 = alloc.allocate(u32_size);
        const void* u32_src = (const void*) u32;
        void* u32_dst = (void*) u32;

        // convert string
        cb1(src, srclen, u32_dst, u32_size, allocator);
        u32_size = distance(u32, (char*) u32_dst);
        cb2(u32_src, u32_size, dst, dstlen, allocator);
    } catch (...) {
        if (u32) {
            alloc.deallocate(u32, u32_size);
        }
        throw;
    }

    // free
    alloc.deallocate(u32, u32_size);
}

// FUNCTIONS
// ---------


void utf8_to_punycode(const void*& src,
    size_t srclen,
    void*& dst,
    size_t dstlen,
    const byte_allocator& allocator)
{
    unicode_lowlevel_callback cb1(utf8_to_utf32);
    punycode_lowlevel_callback cb2(utf32_to_punycode);
    punycode_conversion(src, srclen, dst, dstlen, allocator, cb1, cb2);
}


string utf8_to_punycode(const string_wrapper& str,
    const byte_allocator& allocator)
{
    auto utf32 = utf8_to_utf32(str, allocator);
    return utf32_to_punycode(utf32, allocator);
}


void utf16_to_punycode(const void*& src,
    size_t srclen,
    void*& dst,
    size_t dstlen,
    const byte_allocator& allocator)
{
    unicode_lowlevel_callback cb1(utf16_to_utf32);
    punycode_lowlevel_callback cb2(utf32_to_punycode);
    punycode_conversion(src, srclen, dst, dstlen, allocator, cb1, cb2);
}


string utf16_to_punycode(const string_wrapper& str,
    const byte_allocator& allocator)
{
    auto utf32 = utf16_to_utf32(str, allocator);
    return utf32_to_punycode(utf32, allocator);
}


void utf32_to_punycode(const void*& src,
    size_t srclen,
    void*& dst,
    size_t dstlen,
    const byte_allocator&)
{
    // get preferred formats
    const uint32_t* src_first = (const uint32_t*) src;
    auto* src_last = src_first + (srclen / 4);
    uint8_t* dst_first = (uint8_t*) dst;
    auto* dst_last = dst_first + dstlen;

    // encode
    encode_impl(src_first, src_last, dst_first, dst_last);

    // set pointers
    src = src_first;
    dst = dst_first;
}


string utf32_to_punycode(const string_wrapper& str,
    const byte_allocator& allocator)
{
    // get allocator
    using char_allocator = allocator_traits<byte_allocator>::template rebind_alloc<char>;
    char_allocator alloc(allocator);

    // arguments
    const size_t srclen = str.size();
    const size_t dstlen = srclen * 6 / 4;
    const char* src = str.data();
    char* dst = nullptr;

    try {
        // initialize memory for output
        dst = alloc.allocate(dstlen);
        void* dst_first = static_cast<void*>(dst);
        const void* src_first = static_cast<const void*>(src);

        // create STL container and return
        utf32_to_punycode(src_first, srclen, dst_first, dstlen);
        size_t length = distance(dst, static_cast<char*>(dst_first));
        string output(dst, length, alloc);
        alloc.deallocate(dst, dstlen);

        return output;
    } catch (...) {
        if (dst) {
            alloc.deallocate(dst, dstlen);
        }
        throw;
    }
}


void punycode_to_utf8(const void*& src,
    size_t srclen,
    void*& dst,
    size_t dstlen,
    const byte_allocator& allocator)
{
    punycode_lowlevel_callback cb1(punycode_to_utf32);
    unicode_lowlevel_callback cb2(utf32_to_utf8);
    punycode_conversion(src, srclen, dst, dstlen, allocator, cb1, cb2);
}


string punycode_to_utf8(const string_wrapper& str,
    const byte_allocator& allocator)
{
    auto utf32 = punycode_to_utf32(str, allocator);
    return utf32_to_utf8(utf32, allocator);
}


void punycode_to_utf16(const void*& src,
    size_t srclen,
    void*& dst,
    size_t dstlen,
    const byte_allocator& allocator)
{
    punycode_lowlevel_callback cb1(punycode_to_utf32);
    unicode_lowlevel_callback cb2(utf32_to_utf16);
    punycode_conversion(src, srclen, dst, dstlen, allocator, cb1, cb2);
}


string punycode_to_utf16(const string_wrapper& str,
    const byte_allocator& allocator)
{
    auto utf32 = punycode_to_utf32(str, allocator);
    return utf32_to_utf16(utf32, allocator);
}


void punycode_to_utf32(const void*& src,
    size_t srclen,
    void*& dst,
    size_t dstlen,
    const byte_allocator&)
{
    // get preferred formats
    const char* src_first = (const char*) src;
    uint32_t* dst_first = (uint32_t*) dst;

    // decode
    decode_impl(src_first, srclen, dst_first, dstlen / 4);

    // set pointers
    src = src_first;
    dst = dst_first;
}


string punycode_to_utf32(const string_wrapper& str,
    const byte_allocator& allocator)
{
    // get allocator
    using char_allocator = allocator_traits<byte_allocator>::template rebind_alloc<char>;
    char_allocator alloc(allocator);

    // arguments
    size_t srclen = str.size();
    size_t dstlen = srclen * 4;
    const char* src = str.data();
    uint32_t* dst = nullptr;

    try {
        // initialize memory for output
        dst = reinterpret_cast<uint32_t*>(alloc.allocate(dstlen));
        void* dst_first = static_cast<void*>(dst);
        const void* src_first = static_cast<const void*>(src);

        // create STL container and return
        punycode_to_utf32(src_first, srclen, dst_first, dstlen);
        size_t length = distance((char*) dst, (char*) dst_first);
        string output(reinterpret_cast<char*>(dst), length, alloc);
        alloc.deallocate(reinterpret_cast<char*>(dst), dstlen);

        return output;
    } catch (...) {
        if (dst) {
            alloc.deallocate(reinterpret_cast<char*>(dst), dstlen);
        }
        throw;
    }
}

PYCPP_END_NAMESPACE
