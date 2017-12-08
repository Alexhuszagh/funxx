//  :copyright: (c) 2017 Alex Huszagh.
//  :license: MIT, see licenses/mit.md for more details.
/**
 *  \addtogroup PyCPP
 *  \brief Reference-wrapper like deque.
 *
 *  Stores non-nullable pointers to existing objects, using a deque
 *  wrapper as the underlying structure.
 *
 *  This is quite similar to the EASTL's instrusive deque, where
 *  the user allocates storage for each node.
 */

#pragma once

#include <pycpp/misc/ordering.h>
#include <pycpp/reference/core.h>
#include <deque>

PYCPP_BEGIN_NAMESPACE

namespace sequence_detail
{
// MACROS
// ------

/**
 *  \brief Macro wrapper to automate iterator construction.
 */
#define REFERENCE_DEQUE_ITERATOR(it)                                    \
    iterator(it, [](pointer& p) -> reference                            \
    {                                                                   \
        return *p;                                                      \
    })

/**
 *  \brief Macro wrapper to automate const_iterator construction.
 */
#define REFERENCE_DEQUE_CONST_ITERATOR(it)                              \
    const_iterator(it, [](const_pointer p) -> const_reference           \
    {                                                                   \
        return *p;                                                      \
    })

// DECLARATION
// -----------

/**
 *  \brief Deque wrapper mapping value pointers to values.
 */
template <
    typename T,
    typename Alloc = std::allocator<T*>,
    template <typename, typename> class Container = std::deque
>
struct reference_deque_base
{
    // MEMBER TYPES
    // ------------
    using self_t = reference_deque_base<T, Alloc, Container>;
    using container_type = Container<T*, Alloc>;
    using value_type = T;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using difference_type = std::ptrdiff_t;
    using size_type = size_t;
    using iterator = sequence_iterator_impl<typename container_type::iterator>;
    using const_iterator = sequence_const_iterator_impl<typename container_type::const_iterator>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    // MEMBER FUNCTIONS
    // ----------------

    // CONSTRUCTORS
    reference_deque_base() = default;
    reference_deque_base(size_type n, reference);
    reference_deque_base(const self_t&) = default;
    self_t & operator=(const self_t&) = default;
    reference_deque_base(self_t&&) = default;
    self_t & operator=(self_t&&) = default;

    // ITERATORS
    iterator begin();
    const_iterator begin() const;
    const_iterator cbegin() const;
    iterator end();
    const_iterator end() const;
    const_iterator cend() const;
    reverse_iterator rbegin();
    const_reverse_iterator rbegin() const;
    const_reverse_iterator crbegin() const;
    reverse_iterator rend();
    const_reverse_iterator rend() const;
    const_reverse_iterator crend() const;

    // CAPACITY
    size_type size() const;
    size_type max_size() const;
    bool empty() const noexcept;
    void shrink_to_fit();

    // ELEMENT ACCESS
    reference operator[](size_type n);
    const_reference operator[](size_type n) const;
    reference at(size_type n);
    const_reference at(size_type n) const;
    reference front();
    const_reference front() const;
    reference back();
    const_reference back() const;

    // MODIFIERS
    void assign(size_type n, reference r);
    void push_front(reference r);
    void pop_front();
    void push_back(reference r);
    void pop_back();
    iterator insert(const_iterator position, reference r);
    iterator insert(const_iterator position, size_type n, reference r);
    iterator erase(const_iterator position);
    iterator erase(const_iterator first, const_iterator last);
    void clear();
    void swap(self_t&);

    // RELATIONAL OPERATORS
    bool operator==(const self_t&) const;
    bool operator!=(const self_t&) const;
    bool operator<(const self_t&) const;
    bool operator<=(const self_t&) const;
    bool operator>(const self_t&) const;
    bool operator>=(const self_t&) const;

private:
    container_type deque_;
};

// IMPLEMENTATION
// --------------


template <typename T, typename A, template <typename, typename> class _>
reference_deque_base<T, A, _>::reference_deque_base(size_type n, reference r)
{
    for (size_t i = 0; i < n; ++i) {
        deque_.emplace_back(std::addressof(r));
    }
}


template <typename T, typename A, template <typename, typename> class _>
auto reference_deque_base<T, A, _>::begin() -> iterator
{
    return REFERENCE_DEQUE_ITERATOR(deque_.begin());
}


template <typename T, typename A, template <typename, typename> class _>
auto reference_deque_base<T, A, _>::begin() const -> const_iterator
{
    return REFERENCE_DEQUE_CONST_ITERATOR(deque_.begin());
}


template <typename T, typename A, template <typename, typename> class _>
auto reference_deque_base<T, A, _>::cbegin() const -> const_iterator
{
    return REFERENCE_DEQUE_CONST_ITERATOR(deque_.begin());
}


template <typename T, typename A, template <typename, typename> class _>
auto reference_deque_base<T, A, _>::end() -> iterator
{
    return REFERENCE_DEQUE_ITERATOR(deque_.end());
}


template <typename T, typename A, template <typename, typename> class _>
auto reference_deque_base<T, A, _>::end() const -> const_iterator
{
    return REFERENCE_DEQUE_CONST_ITERATOR(deque_.end());
}


template <typename T, typename A, template <typename, typename> class _>
auto reference_deque_base<T, A, _>::cend() const -> const_iterator
{
    return REFERENCE_DEQUE_CONST_ITERATOR(deque_.end());
}


template <typename T, typename A, template <typename, typename> class _>
auto reference_deque_base<T, A, _>::rbegin() -> reverse_iterator
{
    return reverse_iterator(end());
}


template <typename T, typename A, template <typename, typename> class _>
auto reference_deque_base<T, A, _>::rbegin() const -> const_reverse_iterator
{
    return const_reverse_iterator(end());
}


template <typename T, typename A, template <typename, typename> class _>
auto reference_deque_base<T, A, _>::crbegin() const -> const_reverse_iterator
{
    return const_reverse_iterator(end());
}


template <typename T, typename A, template <typename, typename> class _>
auto reference_deque_base<T, A, _>::rend() -> reverse_iterator
{
    return reverse_iterator(begin());
}


template <typename T, typename A, template <typename, typename> class _>
auto reference_deque_base<T, A, _>::rend() const -> const_reverse_iterator
{
    return const_reverse_iterator(begin());
}


template <typename T, typename A, template <typename, typename> class _>
auto reference_deque_base<T, A, _>::crend() const -> const_reverse_iterator
{
    return const_reverse_iterator(begin());
}


template <typename T, typename A, template <typename, typename> class _>
auto reference_deque_base<T, A, _>::size() const -> size_type
{
    return deque_.size();
}


template <typename T, typename A, template <typename, typename> class _>
auto reference_deque_base<T, A, _>::max_size() const -> size_type
{
    return deque_.max_size();
}


template <typename T, typename A, template <typename, typename> class _>
bool reference_deque_base<T, A, _>::empty() const noexcept
{
    return deque_.empty();
}


template <typename T, typename A, template <typename, typename> class _>
void reference_deque_base<T, A, _>::shrink_to_fit()
{
    deque_.shrink_to_fit();
}


template <typename T, typename A, template <typename, typename> class _>
auto reference_deque_base<T, A, _>::operator[](size_type n) -> reference
{
    return *deque_[n];
}


template <typename T, typename A, template <typename, typename> class _>
auto reference_deque_base<T, A, _>::operator[](size_type n) const -> const_reference
{
    return *deque_[n];
}


template <typename T, typename A, template <typename, typename> class _>
auto reference_deque_base<T, A, _>::at(size_type n) -> reference
{
    return *deque_.at(n);
}


template <typename T, typename A, template <typename, typename> class _>
auto reference_deque_base<T, A, _>::at(size_type n) const -> const_reference
{
    return *deque_.at(n);
}


template <typename T, typename A, template <typename, typename> class _>
auto reference_deque_base<T, A, _>::front() -> reference
{
    return *deque_.front();
}


template <typename T, typename A, template <typename, typename> class _>
auto reference_deque_base<T, A, _>::front() const -> const_reference
{
    return *deque_.front();
}


template <typename T, typename A, template <typename, typename> class _>
auto reference_deque_base<T, A, _>::back() -> reference
{
    return *deque_.back();
}


template <typename T, typename A, template <typename, typename> class _>
auto reference_deque_base<T, A, _>::back() const -> const_reference
{
    return *deque_.back();
}


template <typename T, typename A, template <typename, typename> class _>
void reference_deque_base<T, A, _>::assign(size_type n, reference r)
{
    deque_.assign(n, std::addressof(r));
}


template <typename T, typename A, template <typename, typename> class _>
void reference_deque_base<T, A, _>::push_front(reference r)
{
    deque_.push_front(std::addressof(r));
}


template <typename T, typename A, template <typename, typename> class _>
void reference_deque_base<T, A, _>::pop_front()
{
    deque_.pop_front();
}


template <typename T, typename A, template <typename, typename> class _>
void reference_deque_base<T, A, _>::push_back(reference r)
{
    deque_.push_back(std::addressof(r));
}


template <typename T, typename A, template <typename, typename> class _>
void reference_deque_base<T, A, _>::pop_back()
{
    deque_.pop_back();
}


template <typename T, typename A, template <typename, typename> class _>
auto reference_deque_base<T, A, _>::insert(const_iterator position, reference r) -> iterator
{
    auto distance = std::distance(cbegin(), position);
    auto it = deque_.insert(deque_.cbegin()+distance, std::addressof(r));
    return iterator(it, [](pointer p) -> reference {
        return *p;
    });
}


template <typename T, typename A, template <typename, typename> class _>
auto reference_deque_base<T, A, _>::insert(const_iterator position, size_type n, reference r) -> iterator
{
    auto distance = std::distance(cbegin(), position);
    auto it = deque_.insert(deque_.cbegin()+distance, n, std::addressof(r));
    return iterator(it, [](pointer p) -> reference {
        return *p;
    });
}


template <typename T, typename A, template <typename, typename> class _>
auto reference_deque_base<T, A, _>::erase(const_iterator position) -> iterator
{
    auto distance = std::distance(cbegin(), position);
    auto it = deque_.erase(deque_.cbegin()+distance);
    return iterator(it, [](pointer p) -> reference {
        return *p;
    });
}


template <typename T, typename A, template <typename, typename> class _>
auto reference_deque_base<T, A, _>::erase(const_iterator first, const_iterator last) -> iterator
{
    auto f = std::distance(cbegin(), first);
    auto l = std::distance(cbegin(), last);
    auto it = deque_.erase(deque_.cbegin()+f, deque_.cbegin()+l);
    return iterator(it, [](pointer p) -> reference {
        return *p;
    });
}


template <typename T, typename A, template <typename, typename> class _>
void reference_deque_base<T, A, _>::clear()
{
    deque_.clear();
}


template <typename T, typename A, template <typename, typename> class _>
void reference_deque_base<T, A, _>::swap(self_t& rhs)
{
    std::swap(deque_, rhs.deque_);
}


template <typename T, typename A, template <typename, typename> class _>
bool reference_deque_base<T, A, _>::operator==(const self_t& rhs) const
{
    return size() == rhs.size() && std::equal(begin(), end(), rhs.begin());
}


template <typename T, typename A, template <typename, typename> class _>
bool reference_deque_base<T, A, _>::operator!=(const self_t& rhs) const
{
    return not_equal_to(*this, rhs);
}


template <typename T, typename A, template <typename, typename> class _>
bool reference_deque_base<T, A, _>::operator<(const self_t& rhs) const
{
    return std::lexicographical_compare(begin(), end(), rhs.begin(), rhs.end());
}


template <typename T, typename A, template <typename, typename> class _>
bool reference_deque_base<T, A, _>::operator<=(const self_t& rhs) const
{
    return less_equal(*this, rhs);
}


template <typename T, typename A, template <typename, typename> class _>
bool reference_deque_base<T, A, _>::operator>(const self_t& rhs) const
{
    return greater(*this, rhs);
}


template <typename T, typename A, template <typename, typename> class _>
bool reference_deque_base<T, A, _>::operator>=(const self_t& rhs) const
{
    return greater_equal(*this, rhs);
}

// CLEANUP
// -------

#undef REFERENCE_DEQUE_ITERATOR
#undef REFERENCE_DEQUE_CONST_ITERATOR

}   /* sequence_detail */

// OBJECTS
// -------

/**
 *  \brief Deque wrapper that handles reference values.
 */
template <
    typename T,
    typename Alloc = std::allocator<T*>,
    template <typename, typename> class Container = std::deque
>
using reference_deque = sequence_detail::reference_deque_base<T, Alloc, Container>;

PYCPP_END_NAMESPACE
