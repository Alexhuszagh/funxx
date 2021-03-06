//  :copyright: (c) 2017 Alex Huszagh.
//  :license: MIT, see licenses/mit.md for more details.
/**
 *  \addtogroup PyCPP
 *  \brief BLOSC compression and decompression.
 */

#pragma once

#if defined(HAVE_BLOSC)

#include <pycpp/compression/exception.h>
#include <pycpp/stl/memory.h>
#include <pycpp/string/string.h>

PYCPP_BEGIN_NAMESPACE

// FUNCTIONS
// ---------

/**
 *  \brief BLOSC-compress data. Returns number of bytes converted.
 */
void blosc_compress(const void*& src, size_t srclen, void*& dst, size_t dstlen);

/**
 *  \brief BLOSC-compress data.
 */
string blosc_compress(const string_wrapper& str);

/**
 *  \brief BLOSC-decompress data.
 */
string blosc_decompress(const string_wrapper& str);

/**
 *  \brief BLOSC-decompress data. Returns number of bytes converted.
 *
 *  \param bound            Known size of decompressed buffer.
 */
void blosc_decompress(const void*& src, size_t srclen, void*& dst, size_t dstlen, size_t bound);

/**
 *  \brief BLOSC-decompress data.
 *
 *  \param bound            Known size of decompressed buffer.
 */
string blosc_decompress(const string_wrapper& str, size_t bound);

PYCPP_END_NAMESPACE

#endif                  // HAVE_BLOSC
