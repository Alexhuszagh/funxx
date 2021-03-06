//  :copyright: (c) 2017 Alex Huszagh.
//  :license: MIT, see licenses/mit.md for more details.
/**
 *  \addtogroup PyCPP
 *  \brief Fixed-size map and multimap implementation.
 *
 *  Based on the EASTL, this map and multimap implementation
 *  preallocates a fixed-size buffer on the stack for allocation
 *  up until a certain size, and uses dynamic memory allocation
 *  afterwards.
 */

#pragma once

#include <pycpp/fixed/arena.h>
#include <pycpp/stl/initializer_list.h>
#include <pycpp/stl/map.h>

PYCPP_BEGIN_NAMESPACE

// DECLARATION
// -----------

// map

/**
 *  \brief Fixed-sized map that preallocates memory on the stack.
 */
template <
    typename Key,
    typename T,
    typename Compare = less<Key>,
    size_t StackSize = 4096,
    template <typename, typename,typename, typename> class Container = map
>
struct fixed_map:
    fixed_arena<pair<const Key, T>, StackSize>,
    Container<Key, T, Compare, stack_allocator<pair<const Key, T>, StackSize>>
{
public:
    // MEMBER TYPES
    // ------------
    using self_t = fixed_map<Key, T, Compare, StackSize, Container>;
    using container_type = Container<Key, T, Compare, stack_allocator<pair<const Key, T>, StackSize>>;
    using value_type = typename container_type::value_type;
    using allocator_type = typename container_type::allocator_type;
    using arena_type = typename allocator_type::arena_type;

    // MEMBER FUNCTIONS
    // ----------------
    // TODO: here...

private:
    void reset();
};

// multimap

/**
 *  \brief Fixed-sized multimap that preallocates memory on the stack.
 */
template <
    typename Key,
    typename T,
    typename Compare = less<Key>,
    size_t StackSize = 4096,
    template <typename, typename,typename, typename> class Container = multimap
>
struct fixed_multimap:
    fixed_arena<pair<const Key, T>, StackSize>,
    Container<Key, T, Compare, stack_allocator<pair<const Key, T>, StackSize>>
{
public:
    // MEMBER TYPES
    // ------------
    using self_t = fixed_multimap<Key, T, Compare, StackSize, Container>;
    using container_type = Container<Key, T, Compare, stack_allocator<pair<const Key, T>, StackSize>>;
    using value_type = typename container_type::value_type;
    using allocator_type = typename container_type::allocator_type;
    using arena_type = typename allocator_type::arena_type;

    // MEMBER FUNCTIONS
    // ----------------
    // TODO: here...

private:
    void reset();
};

// SPECIALIZATION
// --------------

template <
    typename Key,
    typename T,
    typename Compare,
    size_t StackSize,
    template <typename, typename,typename, typename> class Container
>
struct is_relocatable<fixed_map<Key, T, Compare, StackSize, Container>>: bool_constant<
        is_relocatable<typename fixed_map<Key, T, Compare, StackSize, Container>::arena_type>::value &&
        is_relocatable<typename fixed_map<Key, T, Compare, StackSize, Container>::container_type>::value
    >
{};

template <
    typename Key,
    typename T,
    typename Compare,
    size_t StackSize,
    template <typename, typename,typename, typename> class Container
>
struct is_relocatable<fixed_multimap<Key, T, Compare, StackSize, Container>>: bool_constant<
        is_relocatable<typename fixed_multimap<Key, T, Compare, StackSize, Container>::arena_type>::value &&
        is_relocatable<typename fixed_multimap<Key, T, Compare, StackSize, Container>::container_type>::value
    >
{};

// IMPLEMENTATION
// --------------

// map


template <typename Key, typename T, typename Compare, size_t StackSize, template <typename, typename,typename, typename> class _>
void fixed_map<Key, T, Compare, StackSize, _>::reset()
{
    // clear the existing container and reset the allocator for efficiency
    // do not reset the arena, since that is **undefined behavior**
    this->clear();
}

// multimap

template <typename Key, typename T, typename Compare, size_t StackSize, template <typename, typename,typename, typename> class _>
void fixed_multimap<Key, T, Compare, StackSize, _>::reset()
{
    // clear the existing container and reset the allocator for efficiency
    // do not reset the arena, since that is **undefined behavior**
    this->clear();
}

PYCPP_END_NAMESPACE
