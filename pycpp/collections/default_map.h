//  :copyright: (c) 2017 Alex Huszagh.
//  :license: MIT, see licenses/mit.md for more details.
/**
 *  \addtogroup PyCPP
 *  \brief Dictionary with function overload for missing values.
 */

 #pragma once

#include <pycpp/stl/initializer_list.h>
#include <pycpp/stl/map.h>
#include <pycpp/stl/unordered_map.h>
#include <assert.h>

PYCPP_BEGIN_NAMESPACE

// ALIAS
// -----

template <typename T>
using default_map_callback = function<T()>;

// DECLARATION
// -----------

/**
 *  \brief Use default constructor to initialize value.
 */
template <typename T>
T default_constructor();

/**
 *  \brief Binary search tree implementation of the default map.
 */
template <
    typename Key,
    typename T,
    typename Compare = less<Key>,
    typename Alloc = allocator<pair<const Key, T>>,
    template <typename, typename, typename, typename> class Map = map
>
struct default_map
{
public:
    // MEMBER TYPES
    // ------------
    using map_type = Map<Key, T, Compare, Alloc>;
    using self_t = default_map<Key, T, Compare, Alloc, Map>;
    using callback_type = default_map_callback<T>;
    using key_type = typename map_type::key_type;
    using mapped_type = typename map_type::mapped_type;
    using value_type = typename map_type::value_type;
    using key_compare = typename map_type::key_compare;
    using value_compare = typename map_type::value_compare;
    using allocator_type = typename map_type::allocator_type;
    using reference = typename map_type::reference;
    using const_reference = typename map_type::const_reference;
    using const_pointer = typename map_type::const_pointer;
    using iterator = typename map_type::iterator;
    using const_iterator = typename map_type::const_iterator;
    using reverse_iterator = typename map_type::reverse_iterator;
    using const_reverse_iterator = typename map_type::const_reverse_iterator;
    using difference_type = typename map_type::difference_type;
    using size_type = typename map_type::size_type;

    // MEMBER FUNCTIONS
    // ----------------
    default_map(callback_type callback = default_constructor<mapped_type>);
    default_map(initializer_list<value_type>, callback_type callback = default_constructor<mapped_type>);
    template <typename Iter> default_map(Iter first, Iter last, callback_type callback = default_constructor<mapped_type>);
    default_map(const self_t&);
    self_t& operator=(const self_t&);
    default_map(self_t&&);
    self_t& operator=(self_t&&);

    // CAPACITY
    bool empty() const noexcept;
    size_type size() const noexcept;
    size_type max_size() const noexcept;

    // ITERATORS
    iterator begin() noexcept;
    const_iterator begin() const noexcept;
    iterator end() noexcept;
    const_iterator end() const noexcept;
    reverse_iterator rbegin() noexcept;
    const_reverse_iterator rbegin() const noexcept;
    reverse_iterator rend() noexcept;
    const_reverse_iterator rend() const noexcept;
    const_iterator cbegin() const noexcept;
    const_iterator cend() const noexcept;
    const_reverse_iterator crbegin() const noexcept;
    const_reverse_iterator crend() const noexcept;

    // ELEMENT ACCESS
    mapped_type& operator[](const key_type& k);
    mapped_type& operator[](key_type&& k);
    mapped_type& at(const key_type& k);
    const mapped_type& at(const key_type& k) const;

    // OPERATIONS
    iterator find(const key_type& k);
    const_iterator find(const key_type& k) const;
    size_type count(const key_type& k) const;
    iterator lower_bound(const key_type& k);
    const_iterator lower_bound(const key_type& k) const;
    iterator upper_bound(const key_type& k);
    const_iterator upper_bound(const key_type& k) const;
    pair<iterator, iterator> equal_range(const key_type& k);
    pair<const_iterator, const_iterator> equal_range(const key_type& k) const;

    // MODIFIERS
    template <typename... Ts> pair<iterator, bool> emplace(Ts&&... ts);
    template <typename... Ts> iterator emplace_hint(const_iterator position, Ts&&... ts);
    pair<iterator, bool> insert(const value_type& val);
    template <typename U> pair<iterator, bool> insert(U&& val);
    iterator insert(const_iterator position, const value_type& val);
    template <typename U> iterator insert(const_iterator position, U&& val);
    template <typename Iter> void insert(Iter first, Iter last);
    void insert(initializer_list<value_type> list);
    iterator erase(const_iterator position);
    size_type erase(const key_type& k);
    iterator erase(const_iterator first, const_iterator last);
    void clear() noexcept;
    void swap(self_t& rhs);

    // OBSERVERS
    allocator_type get_allocator() const noexcept;
    key_compare key_comp() const;
    value_compare value_comp() const;

    // OPERATORS
    bool operator==(const self_t& rhs) const;
    bool operator!=(const self_t& rhs) const;

private:
    map_type map_;
    callback_type callback_ = nullptr;

    // FRIEND
    template <typename K, typename U, typename C, typename A, template <typename, typename, typename, typename> class M>
    friend void swap(const default_map<K, U, C, A, M>& lhs, const default_map<K, U, C, A, M>& rhs);
};


/**
 *  \brief Hashmap implementation of the default map.
 */
template <
    typename Key,
    typename T,
    typename Hash = hash<Key>,
    typename Pred = equal_to<Key>,
    typename Alloc = allocator<pair<const Key, T>>,
    template <typename, typename, typename, typename, typename> class Map = unordered_map
>
struct default_unordered_map
{
public:
    // MEMBER TYPES
    // ------------
    using map_type = Map<Key, T, Hash, Pred, Alloc>;
    using self_t = default_unordered_map<Key, T, Hash, Pred, Alloc, Map>;
    using callback_type = default_map_callback<T>;
    using key_type = typename map_type::key_type;
    using mapped_type = typename map_type::mapped_type;
    using value_type = typename map_type::value_type;
    using hasher = typename map_type::hasher;
    using key_equal = typename map_type::key_equal;
    using allocator_type = typename map_type::allocator_type;
    using reference = typename map_type::reference;
    using const_reference = typename map_type::const_reference;
    using pointer = typename map_type::pointer;
    using const_pointer = typename map_type::const_pointer;
    using iterator = typename map_type::iterator;
    using const_iterator = typename map_type::const_iterator;
    using local_iterator = typename map_type::local_iterator;
    using const_local_iterator = typename map_type::const_local_iterator;
    using size_type = typename map_type::size_type;
    using difference_type = typename map_type::difference_type;

    // MEMBER FUNCTIONS
    // ----------------
    default_unordered_map(callback_type callback = default_constructor<mapped_type>);
    default_unordered_map(initializer_list<value_type>, callback_type callback = default_constructor<mapped_type>);
    template <typename Iter> default_unordered_map(Iter first, Iter last, callback_type callback = default_constructor<mapped_type>);
    default_unordered_map(const self_t&);
    self_t& operator=(const self_t&);
    default_unordered_map(self_t&&);
    self_t& operator=(self_t&&);

    // CAPACITY
    bool empty() const noexcept;
    size_type size() const noexcept;
    size_type max_size() const noexcept;

    // ITERATORS
    iterator begin() noexcept;
    const_iterator begin() const noexcept;
    local_iterator begin(size_type n) noexcept;
    const_local_iterator begin(size_type n) const noexcept;
    iterator end() noexcept;
    const_iterator end() const noexcept;
    local_iterator end(size_type n) noexcept;
    const_local_iterator end(size_type n) const noexcept;
    const_iterator cbegin() const noexcept;
    const_local_iterator cbegin(size_type n) const noexcept;
    const_iterator cend() const noexcept;
    const_local_iterator cend(size_type n) const noexcept;

    // ELEMENT ACCESS
    mapped_type& operator[](const key_type& k);
    mapped_type& operator[](key_type&& k);
    mapped_type& at(const key_type& k);
    const mapped_type& at(const key_type& k) const;

    // OPERATIONS
    iterator find(const key_type& k);
    const_iterator find(const key_type& k) const;
    size_type count(const key_type& k) const;
    pair<iterator, iterator> equal_range(const key_type& k);
    pair<const_iterator, const_iterator> equal_range(const key_type& k) const;

    // MODIFIERS
    template <typename... Ts> pair<iterator, bool> emplace(Ts&&... ts);
    template <typename... Ts> iterator emplace_hint(const_iterator position, Ts&&... ts);
    pair<iterator, bool> insert(const value_type& val);
    template <typename U> pair<iterator, bool> insert(U&& val);
    iterator insert(const_iterator position, const value_type& val);
    template <typename U> iterator insert(const_iterator position, U&& val);
    template <typename Iter> void insert(Iter first, Iter last);
    void insert(initializer_list<value_type> list);
    iterator erase(const_iterator position);
    size_type erase(const key_type& k);
    iterator erase(const_iterator first, const_iterator last);
    void clear() noexcept;
    void swap(self_t& rhs);

    // BUCKETS
    size_type bucket_count() const noexcept;
    size_type max_bucket_count() const noexcept;
    size_type bucket_size(size_type n) const;
    size_type bucket(const key_type& k) const;

    // HASH POLICY
    float load_factor() const noexcept;
    float max_load_factor() const noexcept;
    void max_load_factor(float z);
    void rehash(size_type n);
    void reserve(size_type n);

    // OBSERVERS
    allocator_type get_allocator() const noexcept;
    hasher hash_function() const;
    key_equal key_eq() const;

    // OPERATORS
    bool operator==(const self_t& rhs) const;
    bool operator!=(const self_t& rhs) const;

private:
    map_type map_;
    callback_type callback_ = nullptr;

    // FRIEND
    template <typename K, typename U, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
    friend void swap(const default_unordered_map<K, U, H, P, A, M>& lhs, const default_unordered_map<K, U, H, P, A, M>& rhs);
};

// SPECIALIZATION
// --------------

template <
    typename Key,
    typename T,
    typename Compare,
    typename Alloc,
    template <typename, typename, typename, typename> class Map
>
struct is_relocatable<default_map<Key, T, Compare, Alloc, Map>>:
    is_relocatable<typename default_map<Key, T, Compare, Alloc, Map>::map_type>
{};

template <
    typename Key,
    typename T,
    typename Hash,
    typename Pred,
    typename Alloc,
    template <typename, typename, typename, typename, typename> class Map
>
struct is_relocatable<default_unordered_map<Key, T, Hash, Pred, Alloc, Map>>:
    is_relocatable<typename default_unordered_map<Key, T, Hash, Pred, Alloc, Map>::map_type>
{};

// DEFINITION
// ----------

// FUNCTIONS

template <typename T>
inline T default_constructor()
{
    return T();
}

// DEFAULT MAP

template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
inline default_map<K, T, C, A, M>::default_map(callback_type callback):
    callback_(callback)
{
    assert(callback_ && "Callback cannot be null.");
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
inline default_map<K, T, C, A, M>::default_map(initializer_list<value_type> list, callback_type callback):
    map_(list.begin(), list.end()),
    callback_(callback)
{
    assert(callback_ && "Callback cannot be null.");
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
template <typename Iter>
inline default_map<K, T, C, A, M>::default_map(Iter first, Iter last, callback_type callback):
    map_(first, last),
    callback_(callback)
{
    assert(callback_ && "Callback cannot be null.");
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
inline default_map<K, T, C, A, M>::default_map(const self_t& rhs):
    map_(rhs.map_),
    callback_(rhs.callback_)
{}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
inline auto default_map<K, T, C, A, M>::operator=(const self_t& rhs) -> self_t&
{
    map_ = rhs.map_;
    callback_ = rhs.callback_;
    return *this;
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
inline default_map<K, T, C, A, M>::default_map(self_t&& rhs)
{
    swap(rhs);
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
inline auto default_map<K, T, C, A, M>::operator=(self_t&& rhs) -> self_t&
{
    swap(rhs);
    return *this;
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
inline bool default_map<K, T, C, A, M>::empty() const noexcept
{
    return map_.empty();
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
inline auto default_map<K, T, C, A, M>::size() const noexcept -> size_type
{
    return map_.size();
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
inline auto default_map<K, T, C, A, M>::max_size() const noexcept -> size_type
{
    return map_.max_size();
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
inline auto default_map<K, T, C, A, M>::begin() noexcept -> iterator
{
    return map_.begin();
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
inline auto default_map<K, T, C, A, M>::begin() const noexcept -> const_iterator
{
    return map_.begin();
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
inline auto default_map<K, T, C, A, M>::end() noexcept -> iterator
{
    return map_.end();
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
inline auto default_map<K, T, C, A, M>::end() const noexcept -> const_iterator
{
    return map_.end();
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
inline auto default_map<K, T, C, A, M>::rbegin() noexcept -> reverse_iterator
{
    return map_.rbegin();
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
inline auto default_map<K, T, C, A, M>::rbegin() const noexcept -> const_reverse_iterator
{
    return map_.rbegin();
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
inline auto default_map<K, T, C, A, M>::rend() noexcept -> reverse_iterator
{
    return map_.rend();
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
inline auto default_map<K, T, C, A, M>::rend() const noexcept -> const_reverse_iterator
{
    return map_.rend();
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
inline auto default_map<K, T, C, A, M>::cbegin() const noexcept -> const_iterator
{
    return map_.cbegin();
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
inline auto default_map<K, T, C, A, M>::cend() const noexcept -> const_iterator
{
    return map_.cend();
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
inline auto default_map<K, T, C, A, M>::crbegin() const noexcept -> const_reverse_iterator
{
    return map_.crbegin();
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
inline auto default_map<K, T, C, A, M>::crend() const noexcept -> const_reverse_iterator
{
    return map_.crend();
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
auto default_map<K, T, C, A, M>::operator[](const key_type& k) -> mapped_type&
{
    auto it = map_.find(k);
    if (it == map_.end()) {
        it = map_.emplace_hint(it, k, callback_());
    }
    return it->second;
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
auto default_map<K, T, C, A, M>::operator[](key_type&& k) -> mapped_type&
{
    auto it = map_.find(k);
    if (it == map_.end()) {
        it = map_.emplace_hint(it, forward<key_type>(k), callback_());
    }
    return it->second;
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
inline auto default_map<K, T, C, A, M>::at(const key_type& k) -> mapped_type&
{
    return map_.at(k);
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
inline auto default_map<K, T, C, A, M>::at(const key_type& k) const -> const mapped_type&
{
    return map_.at(k);
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
inline auto default_map<K, T, C, A, M>::find(const key_type& k) -> iterator
{
    return map_.find(k);
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
inline auto default_map<K, T, C, A, M>::find(const key_type& k) const -> const_iterator
{
    return map_.find(k);
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
inline auto default_map<K, T, C, A, M>::count(const key_type& k) const -> size_type
{
    return map_.count(k);
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
inline auto default_map<K, T, C, A, M>::lower_bound(const key_type& k) -> iterator
{
    return map_.lower_bound(k);
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
inline auto default_map<K, T, C, A, M>::lower_bound(const key_type& k) const -> const_iterator
{
    return map_.lower_bound(k);
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
inline auto default_map<K, T, C, A, M>::upper_bound(const key_type& k) -> iterator
{
    return map_.upper_bound(k);
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
inline auto default_map<K, T, C, A, M>::upper_bound(const key_type& k) const -> const_iterator
{
    return map_.upper_bound(k);
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
inline auto default_map<K, T, C, A, M>::equal_range(const key_type& k) -> pair<iterator, iterator>
{
    return map_.equal_range(k);
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
inline auto default_map<K, T, C, A, M>::equal_range(const key_type& k) const -> pair<const_iterator, const_iterator>
{
    return map_.equal_range(k);
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
template <typename... Ts>
inline auto default_map<K, T, C, A, M>::emplace(Ts&&... ts) -> pair<iterator, bool>
{
    return map_.emplace(forward<Ts>(ts)...);
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
template <typename... Ts>
inline auto default_map<K, T, C, A, M>::emplace_hint(const_iterator position, Ts&&... ts) -> iterator
{
    return map_.emplace_hint(position, forward<Ts>(ts)...);
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
inline auto default_map<K, T, C, A, M>::insert(const value_type& val) -> pair<iterator, bool>
{
    return map_.insert(val);
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
template <typename U>
inline auto default_map<K, T, C, A, M>::insert(U&& val) -> pair<iterator, bool>
{
    return map_.insert(forward<U>(val));
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
inline auto default_map<K, T, C, A, M>::insert(const_iterator position, const value_type& val) -> iterator
{
    return map_.insert(position, val);
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
template <typename U>
inline auto default_map<K, T, C, A, M>::insert(const_iterator position, U&& val) -> iterator
{
    return map_.insert(position, forward<U>(val));
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
template <typename Iter>
inline void default_map<K, T, C, A, M>::insert(Iter first, Iter last)
{
    map_.insert(first, last);
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
inline void default_map<K, T, C, A, M>::insert(initializer_list<value_type> list)
{
    map_.insert(list);
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
inline auto default_map<K, T, C, A, M>::erase(const_iterator position) -> iterator
{
    return map_.erase(position);
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
inline auto default_map<K, T, C, A, M>::erase(const key_type& k) -> size_type
{
    return map_.erase(k);
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
inline auto default_map<K, T, C, A, M>::erase(const_iterator first, const_iterator last) -> iterator
{
    return map_.erase(first, last);
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
inline void default_map<K, T, C, A, M>::clear() noexcept
{
    map_.clear();
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
inline void default_map<K, T, C, A, M>::swap(self_t& rhs)
{
    using PYCPP_NAMESPACE::swap;
    swap(map_, rhs.map_);
    swap(callback_, rhs.callback_);
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
inline auto default_map<K, T, C, A, M>::get_allocator() const noexcept -> allocator_type
{
    return map_.get_allocator();
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
inline auto default_map<K, T, C, A, M>::key_comp() const -> key_compare
{
    return map_.key_comp();
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
inline auto default_map<K, T, C, A, M>::value_comp() const -> value_compare
{
    return map_.value_comp();
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
inline bool default_map<K, T, C, A, M>::operator==(const self_t& rhs) const
{
    return map_ == rhs.map_;
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
inline bool default_map<K, T, C, A, M>::operator!=(const self_t& rhs) const
{
    return !operator==(rhs);
}


template <typename K, typename T, typename C, typename A, template <typename, typename, typename, typename> class M>
inline void swap(const default_map<K, T, C, A, M>& lhs, const default_map<K, T, C, A, M>& rhs)
{
    lhs.swap(rhs);
}

// DEFAULT UNORDERED MAP

template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
inline default_unordered_map<K, T, H, P, A, M>::default_unordered_map(callback_type callback):
    callback_(callback)
{
    assert(callback_ && "Callback cannot be null.");
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
inline default_unordered_map<K, T, H, P, A, M>::default_unordered_map(initializer_list<value_type> list, callback_type callback):
    map_(list.begin(), list.end()),
    callback_(callback)
{
    assert(callback_ && "Callback cannot be null.");
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
template <typename Iter>
inline default_unordered_map<K, T, H, P, A, M>::default_unordered_map(Iter first, Iter last, callback_type callback):
    map_(first, last),
    callback_(callback)
{
    assert(callback_ && "Callback cannot be null.");
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
inline default_unordered_map<K, T, H, P, A, M>::default_unordered_map(const self_t& rhs):
    map_(rhs.map_),
    callback_(rhs.callback_)
{}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
inline auto default_unordered_map<K, T, H, P, A, M>::operator=(const self_t& rhs) -> self_t&
{
    map_ = rhs.map_;
    callback_ = rhs.callback_;
    return *this;
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
inline default_unordered_map<K, T, H, P, A, M>::default_unordered_map(self_t&& rhs)
{
    swap(rhs);
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
inline auto default_unordered_map<K, T, H, P, A, M>::operator=(self_t&& rhs) -> self_t&
{
    swap(rhs);
    return *this;
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
inline bool default_unordered_map<K, T, H, P, A, M>::empty() const noexcept
{
    return map_.empty();
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
inline auto default_unordered_map<K, T, H, P, A, M>::size() const noexcept -> size_type
{
    return map_.size();
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
inline auto default_unordered_map<K, T, H, P, A, M>::max_size() const noexcept -> size_type
{
    return map_.max_size();
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
inline auto default_unordered_map<K, T, H, P, A, M>::begin() noexcept -> iterator
{
    return map_.begin();
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
inline auto default_unordered_map<K, T, H, P, A, M>::begin() const noexcept -> const_iterator
{
    return map_.begin();
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
inline auto default_unordered_map<K, T, H, P, A, M>::begin(size_type n) noexcept -> local_iterator
{
    return map_.begin(n);
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
inline auto default_unordered_map<K, T, H, P, A, M>::begin(size_type n) const noexcept -> const_local_iterator
{
    return map_.begin(n);
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
inline auto default_unordered_map<K, T, H, P, A, M>::end() noexcept -> iterator
{
    return map_.end();
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
inline auto default_unordered_map<K, T, H, P, A, M>::end() const noexcept -> const_iterator
{
    return map_.end();
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
inline auto default_unordered_map<K, T, H, P, A, M>::end(size_type n) noexcept -> local_iterator
{
    return map_.end(n);
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
inline auto default_unordered_map<K, T, H, P, A, M>::end(size_type n) const noexcept -> const_local_iterator
{
    return map_.end(n);
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
inline auto default_unordered_map<K, T, H, P, A, M>::cbegin() const noexcept -> const_iterator
{
    return map_.cbegin();
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
inline auto default_unordered_map<K, T, H, P, A, M>::cbegin(size_type n) const noexcept -> const_local_iterator
{
    return map_.cbegin(n);
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
inline auto default_unordered_map<K, T, H, P, A, M>::cend() const noexcept -> const_iterator
{
    return map_.cend();
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
inline auto default_unordered_map<K, T, H, P, A, M>::cend(size_type n) const noexcept -> const_local_iterator
{
    return map_.cend(n);
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
auto default_unordered_map<K, T, H, P, A, M>::operator[](const key_type& k) -> mapped_type&
{
    auto it = map_.find(k);
    if (it == map_.end()) {
        it = map_.emplace_hint(it, k, callback_());
    }
    return it->second;
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
auto default_unordered_map<K, T, H, P, A, M>::operator[](key_type&& k) -> mapped_type&
{
    auto it = map_.find(k);
    if (it == map_.end()) {
        it = map_.emplace_hint(it, forward<key_type>(k), callback_());
    }
    return it->second;
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
inline auto default_unordered_map<K, T, H, P, A, M>::at(const key_type& k) -> mapped_type&
{
    return map_.at(k);
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
inline auto default_unordered_map<K, T, H, P, A, M>::at(const key_type& k) const -> const mapped_type&
{
    return map_.at(k);
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
inline auto default_unordered_map<K, T, H, P, A, M>::find(const key_type& k) -> iterator
{
    return map_.find(k);
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
inline auto default_unordered_map<K, T, H, P, A, M>::find(const key_type& k) const -> const_iterator
{
    return map_.find(k);
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
inline auto default_unordered_map<K, T, H, P, A, M>::count(const key_type& k) const -> size_type
{
    return map_.count(k);
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
inline auto default_unordered_map<K, T, H, P, A, M>::equal_range(const key_type& k) -> pair<iterator, iterator>
{
    return map_.equal_range(k);
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
inline auto default_unordered_map<K, T, H, P, A, M>::equal_range(const key_type& k) const -> pair<const_iterator, const_iterator>
{
    return map_.equal_range(k);
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
template <typename... Ts>
inline auto default_unordered_map<K, T, H, P, A, M>::emplace(Ts&&... ts) -> pair<iterator, bool>
{
    return map_.emplace(forward<Ts>(ts)...);
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
template <typename... Ts>
inline auto default_unordered_map<K, T, H, P, A, M>::emplace_hint(const_iterator position, Ts&&... ts) -> iterator
{
    return map_.emplace_hint(position, forward<Ts>(ts)...);
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
inline auto default_unordered_map<K, T, H, P, A, M>::insert(const value_type& val) -> pair<iterator, bool>
{
    return map_.insert(val);
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
template <typename U>
inline auto default_unordered_map<K, T, H, P, A, M>::insert(U&& val) -> pair<iterator, bool>
{
    return map_.insert(forward<U>(val));
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
inline auto default_unordered_map<K, T, H, P, A, M>::insert(const_iterator position, const value_type& val) -> iterator
{
    return map_.insert(position, val);
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
template <typename U>
inline auto default_unordered_map<K, T, H, P, A, M>::insert(const_iterator position, U&& val) -> iterator
{
    return map_.insert(position, forward<U>(val));
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
template <typename Iter>
inline void default_unordered_map<K, T, H, P, A, M>::insert(Iter first, Iter last)
{
    map_.insert(first, last);
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
inline void default_unordered_map<K, T, H, P, A, M>::insert(initializer_list<value_type> list)
{
    map_.insert(list);
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
inline auto default_unordered_map<K, T, H, P, A, M>::erase(const_iterator position) -> iterator
{
    return map_.erase(position);
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
inline auto default_unordered_map<K, T, H, P, A, M>::erase(const key_type& k) -> size_type
{
    return map_.erase(k);
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
inline auto default_unordered_map<K, T, H, P, A, M>::erase(const_iterator first, const_iterator last) -> iterator
{
    return map_.erase(first, last);
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
inline void default_unordered_map<K, T, H, P, A, M>::clear() noexcept
{
    map_.clear();
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
inline void default_unordered_map<K, T, H, P, A, M>::swap(self_t& rhs)
{
    using PYCPP_NAMESPACE::swap;
    swap(map_, rhs.map_);
    swap(callback_, rhs.callback_);
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
inline auto default_unordered_map<K, T, H, P, A, M>::bucket_count() const noexcept -> size_type
{
    return map_.bucket_count();
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
inline auto default_unordered_map<K, T, H, P, A, M>::max_bucket_count() const noexcept -> size_type
{
    return map_.max_bucket_count();
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
inline auto default_unordered_map<K, T, H, P, A, M>::bucket_size(size_type n) const -> size_type
{
    return map_.bucket_size(n);
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
inline auto default_unordered_map<K, T, H, P, A, M>::bucket(const key_type& k) const -> size_type
{
    return map_.bucket(k);
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
inline auto default_unordered_map<K, T, H, P, A, M>::load_factor() const noexcept -> float
{
    return map_.load_factor();
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
inline auto default_unordered_map<K, T, H, P, A, M>::max_load_factor() const noexcept -> float
{
    return map_.max_load_factor();
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
inline auto default_unordered_map<K, T, H, P, A, M>::max_load_factor(float z) -> void
{
    return map_.max_load_factor(z);
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
inline auto default_unordered_map<K, T, H, P, A, M>::rehash(size_type n) -> void
{
    return map_.rehash(n);
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
inline auto default_unordered_map<K, T, H, P, A, M>::reserve(size_type n) -> void
{
    return map_.reserve(n);
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
inline auto default_unordered_map<K, T, H, P, A, M>::get_allocator() const noexcept -> allocator_type
{
    return map_.get_allocator();
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
inline auto default_unordered_map<K, T, H, P, A, M>::hash_function() const -> hasher
{
    return map_.hash_function();
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
inline auto default_unordered_map<K, T, H, P, A, M>::key_eq() const -> key_equal
{
    return map_.key_eq();
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
inline bool default_unordered_map<K, T, H, P, A, M>::operator==(const self_t& rhs) const
{
    return map_ == rhs.map_;
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
inline bool default_unordered_map<K, T, H, P, A, M>::operator!=(const self_t& rhs) const
{
    return !operator==(rhs);
}


template <typename K, typename T, typename H, typename P, typename A, template <typename, typename, typename, typename, typename> class M>
inline void swap(const default_unordered_map<K, T, H, P, A, M>& lhs, const default_unordered_map<K, T, H, P, A, M>& rhs)
{
    lhs.swap(rhs);
}

PYCPP_END_NAMESPACE
