//  :copyright: (c) 2017 Alex Huszagh.
//  :license: MIT, see licenses/mit.md for more details.
/**
 *  \addtogroup PyCPP
 *  \brief Least-recently used cache.
 */

#pragma once

#include <pycpp/reference/core.h>
#include <functional>
#include <list>
#include <unordered_map>

PYCPP_BEGIN_NAMESPACE

namespace detail
{
// DECLARATION
// -----------

template <typename it>
using iterator_value_type = typename std::iterator_traits<it>::value_type;

template <typename it>
using lru_iterator_transform = typename iterator_value_type<it>::second_type&;

template <typename it>
using lru_cache_transform = std::function<lru_iterator_transform<it>(iterator_value_type<it>&)>;

template <typename it>
using lru_iterator = transform_iterator<it, lru_cache_transform<it>>;

template <typename it>
using lru_iterator_const_transform = const typename iterator_value_type<it>::second_type&;

template <typename it>
using lru_cache_const_transform = std::function<lru_iterator_const_transform<it>(const iterator_value_type<it>&)>;

template <typename it>
using lru_const_iterator = transform_iterator<it, lru_cache_const_transform<it>>;

template <typename lru>
using lru_list = std::list<typename lru::value_type, typename lru::allocator_type>;

template <typename lru>
using lru_map = std::unordered_map<typename lru::key_type, typename lru_list<lru>::iterator, typename lru::hasher, typename lru::key_equal>;

}   /* detail */

// MACROS
// ------

/**
 *  \brief Macro wrapper to automate iterator construction.
 */
#define LRU_ITERATOR(it)                                                \
    iterator(it, [](value_type& p) -> mapped_type&                      \
    {                                                                   \
        return p.second;                                                \
    })

/**
 *  \brief Macro wrapper to automate const_iterator construction.
 */
#define LRU_CONST_ITERATOR(it)                                          \
    const_iterator(it, [](const value_type& p) -> const mapped_type&    \
    {                                                                   \
        return p.second;                                                \
    })

// DECLARATION
// -----------

/**
 *  \brief O(1) LRU cache implemented via a hashtable and linked list.
 */
template <
    typename Key,
    typename Value,
    typename Hash = std::hash<Key>,
    typename Pred = std::equal_to<Key>,
    typename Alloc = std::allocator<std::pair<const Key, Value>>
>
struct lru_cache
{
public:
    // MEMBER TYPES
    // ------------
    using self = lru_cache<Key, Value, Hash, Pred, Alloc>;
    using key_type = Key;
    using mapped_type = Value;
    using value_type = std::pair<const key_type, mapped_type>;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using hasher = Hash;
    using key_equal = Pred;
    using allocator_type = Alloc;
    using size_type = size_t;
    using difference_type = std::ptrdiff_t;
    using iterator = detail::lru_iterator<typename detail::lru_list<self>::iterator>;
    using const_iterator = detail::lru_const_iterator<typename detail::lru_list<self>::iterator>;

    // MEMBER FUNCTIONS
    // ----------------
    lru_cache(int cache_size);
    lru_cache(const self&);
    self& operator=(const self&);
    lru_cache(self&&);
    self& operator=(self&&);

    // CAPACITY
    size_type size() const;
    size_type cache_size() const;
    size_type max_size() const;
    bool empty() const noexcept;

    // ITERATORS
    iterator begin();
    const_iterator begin() const;
    const_iterator cbegin() const;
    iterator end();
    const_iterator end() const;
    const_iterator cend() const;

    // ELEMENT ACCESS
    mapped_type& operator[](const key_type&);
    mapped_type& operator[](key_type&&);
    mapped_type& at(const key_type&);
    const mapped_type& at(const key_type&) const;

    // ELEMENT LOOKUP
    iterator find(const key_type&);
    const_iterator find(const key_type&) const;
    size_type count(const key_type&) const;
    std::pair<iterator, iterator> equal_range(const key_type&);
    std::pair<const_iterator, const_iterator> equal_range(const key_type&) const;

    // MODIFIERS
    std::pair<iterator, bool> insert(const key_type&, const mapped_type&);
    std::pair<iterator, bool> insert(const key_type&, mapped_type&&);
    iterator erase(const_iterator);
    size_type erase(const key_type&);
    iterator erase(const_iterator, const_iterator);
    void clear();
    void swap(self&);

    // BUCKET
    size_type bucket_count() const noexcept;
    size_type max_bucket_count() const noexcept;
    size_type bucket_size(size_type) const;
    size_type bucket(const key_type&) const;

    // HASH POLICY
    float load_factor() const noexcept;
    float max_load_factor() const noexcept;
    void max_load_factor(float);
    void rehash(size_type);
    void reserve(size_type);

    // OBSERVERS
    hasher hash_function() const;
    key_equal key_eq() const;
    allocator_type get_allocator() const;

protected:
    using list_t = detail::lru_list<self>;
    using list_iterator_t = typename list_t::iterator;
    using map_t = detail::lru_map<self>;

    // CACHE
    void clean();
    iterator pop(const_iterator);
    iterator put(const key_type&, const mapped_type&);
    iterator put(const key_type&, mapped_type&&);
    iterator get(iterator);
    const_iterator get(const_iterator) const;

    mutable list_t list_;
    map_t map_;
    size_type cache_size_;
};

// IMPLEMENTATION
// --------------

template <typename K, typename V, typename H, typename P, typename A>
lru_cache<K, V, H, P, A>::lru_cache(int cache_size):
    cache_size_(cache_size)
{}


template <typename K, typename V, typename H, typename P, typename A>
lru_cache<K, V, H, P, A>::lru_cache(const self& rhs):
    cache_size_(rhs.cache_size_),
    list_(rhs.list_)
{
    for (auto it = list_.begin(); it != list_.end(); ++it) {
        map_.insert(std::make_pair(it->first, it));
    }
}


template <typename K, typename V, typename H, typename P, typename A>
auto lru_cache<K, V, H, P, A>::operator=(const self& rhs) -> self&
{
    cache_size_ = rhs.cache_size_;
    for (const value_type& value: rhs.list_) {
        list_.emplace_back(std::make_pair(value.first, value.second));
        map_.insert(std::make_pair(value.first, list_.rbegin().base()));
    }

    return *this;
}


template <typename K, typename V, typename H, typename P, typename A>
lru_cache<K, V, H, P, A>::lru_cache(self&& rhs)
{
    swap(rhs);
}


template <typename K, typename V, typename H, typename P, typename A>
auto lru_cache<K, V, H, P, A>::operator=(self&& rhs) -> self&
{
    swap(rhs);
    return *this;
}


template <typename K, typename V, typename H, typename P, typename A>
auto lru_cache<K, V, H, P, A>::cache_size() const -> size_type
{
    return cache_size_;
}


template <typename K, typename V, typename H, typename P, typename A>
auto lru_cache<K, V, H, P, A>::size() const -> size_type
{
    return map_.size();
}


template <typename K, typename V, typename H, typename P, typename A>
auto lru_cache<K, V, H, P, A>::max_size() const -> size_type
{
    return map_.max_size();
}


template <typename K, typename V, typename H, typename P, typename A>
bool lru_cache<K, V, H, P, A>::empty() const noexcept
{
    return map_.empty();
}


template <typename K, typename V, typename H, typename P, typename A>
auto lru_cache<K, V, H, P, A>::begin() -> iterator
{
    return LRU_ITERATOR(list_.begin());
}


template <typename K, typename V, typename H, typename P, typename A>
auto lru_cache<K, V, H, P, A>::begin() const -> const_iterator
{
    return LRU_CONST_ITERATOR(list_.begin());
}


template <typename K, typename V, typename H, typename P, typename A>
auto lru_cache<K, V, H, P, A>::cbegin() const -> const_iterator
{
    return LRU_CONST_ITERATOR(list_.begin());
}


template <typename K, typename V, typename H, typename P, typename A>
auto lru_cache<K, V, H, P, A>::end() -> iterator
{
    return LRU_ITERATOR(list_.end());
}


template <typename K, typename V, typename H, typename P, typename A>
auto lru_cache<K, V, H, P, A>::end() const -> const_iterator
{
    return LRU_CONST_ITERATOR(list_.end());
}


template <typename K, typename V, typename H, typename P, typename A>
auto lru_cache<K, V, H, P, A>::cend() const -> const_iterator
{
    return LRU_CONST_ITERATOR(list_.end());
}


template <typename K, typename V, typename H, typename P, typename A>
auto lru_cache<K, V, H, P, A>::operator[](const key_type& key) -> mapped_type&
{
    auto it = map_.find(key);
    if (it == map_.end()) {
        return *put(key, mapped_type());
    }

    return *get(LRU_ITERATOR(it->second));
}


template <typename K, typename V, typename H, typename P, typename A>
auto lru_cache<K, V, H, P, A>::operator[](key_type&& key) -> mapped_type&
{
    auto it = map_.find(key);
    if (it == map_.end()) {
        return *put(key, mapped_type());
    }

    return *get(LRU_ITERATOR(it->second));
}


template <typename K, typename V, typename H, typename P, typename A>
auto lru_cache<K, V, H, P, A>::at(const key_type& key) -> mapped_type&
{
    auto it = map_.find(key);
    if (it == map_.end()) {
        throw std::out_of_range("lru_cache::at():: Key not found.");
    }

    return *get(LRU_ITERATOR(it->second));
}


template <typename K, typename V, typename H, typename P, typename A>
auto lru_cache<K, V, H, P, A>::at(const key_type& key) const -> const mapped_type&
{
    auto it = map_.find(key);
    if (it == map_.end()) {
        throw std::out_of_range("lru_cache::at():: Key not found.");
    }

    return *get(LRU_CONST_ITERATOR(it->second));
}


template <typename K, typename V, typename H, typename P, typename A>
auto lru_cache<K, V, H, P, A>::find(const key_type& key) -> iterator
{
    auto it = map_.find(key);
    if (it == map_.end()) {
        return end();
    }

    return get(LRU_ITERATOR(it->second));
}


template <typename K, typename V, typename H, typename P, typename A>
auto lru_cache<K, V, H, P, A>::find(const key_type& key) const -> const_iterator
{
    auto it = map_.find(key);
    if (it == map_.cend()) {
        return cend();
    }

    return get(LRU_CONST_ITERATOR(it->second));
}


template <typename K, typename V, typename H, typename P, typename A>
auto lru_cache<K, V, H, P, A>::count(const key_type& key) const -> size_type
{
    return map_.count(key);
}


template <typename K, typename V, typename H, typename P, typename A>
auto lru_cache<K, V, H, P, A>::equal_range(const key_type& key) -> std::pair<iterator, iterator>
{
    auto pair = map_.equal_range(key);
    if (pair.first == map_.end()) {
        return std::make_pair(end(), end());
    } else if (pair.second == map_.end()) {
        return std::make_pair(get(LRU_ITERATOR(pair.first->second)), end());
    } else {
        return std::make_pair(get(LRU_ITERATOR(pair.first->second)), get(LRU_ITERATOR(pair.second->second)));
    }
}


template <typename K, typename V, typename H, typename P, typename A>
auto lru_cache<K, V, H, P, A>::equal_range(const key_type& key) const -> std::pair<const_iterator, const_iterator>
{
    auto pair = map_.equal_range(key);
    if (pair.first == map_.cend()) {
        return std::make_pair(cend(), cend());
    } else if (pair.second == map_.cend()) {
        return std::make_pair(get(LRU_CONST_ITERATOR(pair.first->second)), cend());
    } else {
        return std::make_pair(get(LRU_CONST_ITERATOR(pair.first->second)), get(LRU_CONST_ITERATOR(pair.second->second)));
    }
}


template <typename K, typename V, typename H, typename P, typename A>
auto lru_cache<K, V, H, P, A>::insert(const key_type& key, const mapped_type& value) -> std::pair<iterator, bool>
{
    auto it = map_.find(key);
    if (it == map_.end()) {
        return std::make_pair(put(key, value), true);
    }

    return std::make_pair(LRU_ITERATOR(it->second), false);
}


template <typename K, typename V, typename H, typename P, typename A>
auto lru_cache<K, V, H, P, A>::insert(const key_type& key, mapped_type&& value) -> std::pair<iterator, bool>
{
    auto it = map_.find(key);
    if (it == map_.end()) {
        return std::make_pair(put(key, std::forward<mapped_type>(value)), true);
    }

    return std::make_pair(LRU_ITERATOR(it->second), false);
}


template <typename K, typename V, typename H, typename P, typename A>
auto lru_cache<K, V, H, P, A>::erase(const_iterator pos) -> iterator
{
    return pop(pos);
}


template <typename K, typename V, typename H, typename P, typename A>
auto lru_cache<K, V, H, P, A>::erase(const key_type& key) -> size_type
{
    auto it = map_.find(key);
    if (it == map_.cend()) {
        return 0;
    }
    erase(LRU_CONST_ITERATOR(it->second));
    return 1;
}


template <typename K, typename V, typename H, typename P, typename A>
auto lru_cache<K, V, H, P, A>::erase(const_iterator first, const_iterator last) -> iterator
{
    for (; first != last; ) {
        first = LRU_CONST_ITERATOR(erase(first).base());
    }
    return LRU_ITERATOR(last.base());
}


template <typename K, typename V, typename H, typename P, typename A>
void lru_cache<K, V, H, P, A>::clear()
{
    map_.clear();
    list_.clear();
}


template <typename K, typename V, typename H, typename P, typename A>
void lru_cache<K, V, H, P, A>::swap(self& rhs)
{
    std::swap(list_, rhs.list_);
    std::swap(map_, rhs.map_);
    std::swap(cache_size_, rhs.cache_size_);
}


template <typename K, typename V, typename H, typename P, typename A>
auto lru_cache<K, V, H, P, A>::bucket_count() const noexcept -> size_type
{
    return map_.bucket_count();
}


template <typename K, typename V, typename H, typename P, typename A>
auto lru_cache<K, V, H, P, A>::max_bucket_count() const noexcept -> size_type
{
    return map_.max_bucket_count();
}


template <typename K, typename V, typename H, typename P, typename A>
auto lru_cache<K, V, H, P, A>::bucket_size(size_type n) const -> size_type
{
    return map_.bucket_size(n);
}


template <typename K, typename V, typename H, typename P, typename A>
auto lru_cache<K, V, H, P, A>::bucket(const key_type& key) const -> size_type
{
    return map_.bucket(key);
}


template <typename K, typename V, typename H, typename P, typename A>
float lru_cache<K, V, H, P, A>::load_factor() const noexcept
{
    return map_.load_factor();
}


template <typename K, typename V, typename H, typename P, typename A>
float lru_cache<K, V, H, P, A>::max_load_factor() const noexcept
{
    return map_.max_load_factor();
}


template <typename K, typename V, typename H, typename P, typename A>
void lru_cache<K, V, H, P, A>::max_load_factor(float n)
{
    map_.max_load_factor(n);
}


template <typename K, typename V, typename H, typename P, typename A>
void lru_cache<K, V, H, P, A>::rehash(size_type n)
{
    map_.rehash(n);
}


template <typename K, typename V, typename H, typename P, typename A>
void lru_cache<K, V, H, P, A>::reserve(size_type n)
{
    map_.reserve(n);
}


template <typename K, typename V, typename H, typename P, typename A>
auto lru_cache<K, V, H, P, A>::hash_function() const -> hasher
{
    return hasher();
}


template <typename K, typename V, typename H, typename P, typename A>
auto lru_cache<K, V, H, P, A>::key_eq() const -> key_equal
{
    return key_equal();
}


template <typename K, typename V, typename H, typename P, typename A>
auto lru_cache<K, V, H, P, A>::get_allocator() const -> allocator_type
{
    return allocator_type();
}


template <typename K, typename V, typename H, typename P, typename A>
void lru_cache<K, V, H, P, A>::clean()
{
    while(map_.size() > cache_size()) {
        pop(LRU_CONST_ITERATOR(--list_.end()));
    }
}


template <typename K, typename V, typename H, typename P, typename A>
auto lru_cache<K, V, H, P, A>::pop(const_iterator it) -> iterator
{
    map_.erase(it.base()->first);
    return LRU_ITERATOR(list_.erase(it.base()));
}


template <typename K, typename V, typename H, typename P, typename A>
auto lru_cache<K, V, H, P, A>::put(const key_type& key, const mapped_type& value) -> iterator
{
    list_.push_front(std::make_pair(key, value));
    map_.insert(std::make_pair(key, list_.begin()));
    clean();

    return LRU_ITERATOR(list_.begin());
}


template <typename K, typename V, typename H, typename P, typename A>
auto lru_cache<K, V, H, P, A>::put(const key_type& key, mapped_type&& value) -> iterator
{
    list_.push_front(std::make_pair(key, std::forward<mapped_type>(value)));
    map_.insert(std::make_pair(key, list_.begin()));
    clean();

    return LRU_ITERATOR(list_.begin());
}


template <typename K, typename V, typename H, typename P, typename A>
auto lru_cache<K, V, H, P, A>::get(iterator it) -> iterator
{
    list_.splice(list_.begin(), list_, it.base());
    return it;
}


template <typename K, typename V, typename H, typename P, typename A>
auto lru_cache<K, V, H, P, A>::get(const_iterator it) const -> const_iterator
{
    list_.splice(list_.begin(), list_, it.base());
    return it;
}

// CLEANUP
// -------

#undef LRU_ITERATOR
#undef LRU_CONST_ITERATOR

PYCPP_END_NAMESPACE