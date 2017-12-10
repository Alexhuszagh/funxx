//  :copyright: (c) 2017 Alex Huszagh.
//  :license: MIT, see licenses/mit.md for more details.
/**
 *  \addtogroup PyCPP
 *  \brief Hash specializations for <bitset>.
 */

#pragma once

#include <pycpp/config.h>
#include <bitset>
#include <functional>

PYCPP_BEGIN_NAMESPACE

// FORWARD
// -------

template <typename Key>
struct hash;

// SPECIALIZATION
// --------------

#if USE_XXHASH

template <size_t Size>
struct hash<std::bitset<Size>>
{
    inline size_t operator()(const std::bitset<Size>& x) const noexcept
    {
        return std::hash<std::bitset<Size>>()(x);
    }
};

#endif          // USE_XXHASH

PYCPP_END_NAMESPACE
