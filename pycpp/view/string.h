//  :copyright: (c) 2009-2017 LLVM Team.
//  :copyright: (c) 2017 Alex Huszagh.
//  :license: MIT, see licenses/mit.md for more details.
/**
 *  \addtogroup PyCPP
 *  \brief Zero-copy string view container.
 */

#pragma once

#include <pycpp/config.h>
#include <pycpp/string/find.h>
#include <cassert>
#include <stdexcept>
#include <string>

PYCPP_BEGIN_NAMESPACE

// DECLARATION
// -----------


/**
 *  \brief STL string wrapper.
 *
 *  Binds a pointer and the string length, accepting either a C++
 *  string, a null-terminated string, a character array and length,
 *  or an begin/end pointer pair.
 *
 *  \warning The lifetime of the source data must outlive the wrapper.
 *  The wrapper has **no** ownership, and is merely an STL-like wrapper
 *  for performance reasons.
 */
template <typename Char, typename Traits = std::char_traits<Char>>
class basic_string_view
{
public:
    // MEMBER TEMPLATES
    template <typename C, typename T>
    using view = basic_string_view<C, T>;

    template <typename C, typename T>
    using string = std::basic_string<C, T>;

    template <typename C, typename T>
    using istream = std::basic_istream<C, T>;

    template <typename C, typename T>
    using ostream = std::basic_ostream<C, T>;

    // MEMBER TYPES
    // ------------
    typedef basic_string_view<Char, Traits> self;
    typedef std::basic_string<Char, Traits> stl_type;
    typedef Char value_type;
    typedef Traits traits_type;
    typedef Char& reference;
    typedef const Char& const_reference;
    typedef Char* pointer;
    typedef const Char* const_pointer;
    typedef std::ptrdiff_t difference_type;
    typedef size_t size_type;
    typedef pointer iterator;
    typedef const_pointer const_iterator;
    typedef std::reverse_iterator<pointer> reverse_iterator;
    typedef std::reverse_iterator<const_pointer> const_reverse_iterator;

    // MEMBER VARIABLES
    // ----------------
    static const size_type npos = SIZE_MAX;

    // MEMBER FUNCTIONS
    // ----------------
    basic_string_view() = default;
    basic_string_view(const self& str);
    self& operator=(const self& str);
    basic_string_view(self&& str);
    self& operator=(self&& str);

    basic_string_view(const stl_type& str);
    basic_string_view(const self& str, size_type pos, size_type len = npos);
    basic_string_view(const stl_type& str, size_type pos, size_type len = npos);
    basic_string_view(const_pointer str);
    basic_string_view(const_pointer str, size_type n);
    basic_string_view(const_pointer begin, const_pointer end);
    self& operator=(const_pointer str);

    // ITERATORS
    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;
    const_reverse_iterator rbegin() const;
    const_reverse_iterator rend() const;
    const_iterator cbegin() const;
    const_iterator cend() const;
    const_reverse_iterator crbegin() const;
    const_reverse_iterator crend() const;

    // CAPACITY
    size_type size() const;
    size_type length() const;
    bool empty() const noexcept;

    // ELEMENT ACCESS
    reference operator[](size_type pos);
    const_reference operator[](size_type pos) const;
    reference at(size_type pos);
    const_reference at(size_type pos) const;
    reference front();
    const_reference front() const;
    reference back();
    const_reference back() const;

    // MODIFIERS
    self& assign(const self& str);
    self& assign(const stl_type& str);
    self& assign(const self& str, size_type subpos, size_type sublen);
    self& assign(const stl_type& str, size_type subpos, size_type sublen);
    self& assign(const_pointer s);
    self& assign(const_pointer s, size_type n);
    void swap(self& other);

    // OPERATORS
    self& operator++();
    self operator++(int);
    self& operator--();
    self operator--(int);
    self& operator+=(size_type shift);
    self operator+(size_type shift);
    self& operator-=(size_type shift);
    self operator-(size_type shift);

    // STRING OPERATIONS
    const_pointer c_str() const noexcept;
    const_pointer data() const noexcept;

    // FIND
    size_type find(const self& str, size_type pos = 0) const noexcept;
    size_type find(const stl_type &str, size_type pos = 0) const;
    size_type find(const_pointer array, size_type pos = 0) const;
    size_type find(const_pointer cstring, size_type pos, size_type length) const;
    size_type find(value_type c, size_type pos = 0) const noexcept;

    // FIND FIRST OF
    size_type find_first_of(const self& str, size_type pos = 0) const noexcept;
    size_type find_first_of(const stl_type &str, size_type pos = 0) const;
    size_type find_first_of(const_pointer array, size_type pos = 0) const;
    size_type find_first_of(const_pointer cstring, size_type pos, size_type length) const;
    size_type find_first_of(value_type c, size_type pos = 0) const noexcept;

    // FIND FIRST NOT OF
    size_type find_first_not_of(const self& str, size_type pos = 0) const noexcept;
    size_type find_first_not_of(const stl_type& str, size_type pos = 0) const;
    size_type find_first_not_of(const_pointer array, size_type pos = 0) const;
    size_type find_first_not_of(const_pointer cstring, size_type pos, size_type length) const;
    size_type find_first_not_of(value_type c, size_type pos = 0) const noexcept;

    // RFIND
    size_type rfind(const self& str, size_type pos = 0) const noexcept;
    size_type rfind(const stl_type& str, size_type pos = 0) const;
    size_type rfind(const_pointer array, size_type pos = 0) const;
    size_type rfind(const_pointer cstring, size_type pos, size_type length) const;
    size_type rfind(value_type c, size_type pos = 0) const noexcept;

    // FIND LAST OF
    size_type find_last_of(const self& str, size_type pos = 0) const noexcept;
    size_type find_last_of(const stl_type& str, size_type pos = 0) const;
    size_type find_last_of(const_pointer array, size_type pos = 0) const;
    size_type find_last_of(const_pointer cstring, size_type pos, size_type length) const;
    size_type find_last_of(value_type c, size_type pos = 0) const noexcept;

    // FIND LAST NOT OF
    size_type find_last_not_of(const self& str, size_type pos = 0) const noexcept;
    size_type find_last_not_of(const stl_type &str, size_type pos = 0) const;
    size_type find_last_not_of(const_pointer array, size_type pos = 0) const;
    size_type find_last_not_of(const_pointer cstring, size_type pos, size_type length) const;
    size_type find_last_not_of(value_type c, size_type pos = 0) const noexcept;

    // COMPARE
    int compare(const self& str) const noexcept;
    int compare(const stl_type& str) const noexcept;
    int compare(size_type pos, size_type len, const self& str) const;
    int compare(size_type pos, size_type len, const stl_type& str) const;
    int compare(size_type pos, size_type len, const self& str, size_type subpos, size_type sublen) const;
    int compare(size_type pos, size_type len, const stl_type& str, size_type subpos, size_type sublen) const;
    int compare(const_pointer s) const;
    int compare(size_type pos, size_type len, const_pointer s) const;
    int compare(size_type pos, size_type len, const_pointer s, size_type n) const;

    self substr(size_type pos = 0, size_type len = npos) const;

    // CONVERSIONS
    explicit operator bool() const;
    explicit operator stl_type() const;

private:
    const_pointer data_ = nullptr;
    size_t length_ = 0;

    // NON-MEMBER FUNCTIONS
    // --------------------
    template <typename C, typename T>
    friend void swap(view<C, T>& lhs, view<C, T>& rhs);

    template <typename C, typename T>
    friend istream<C, T>& operator>>(istream<C, T>& stream, view<C, T>& str);

    template <typename C, typename T>
    friend ostream<C, T>& operator<<(ostream<C, T>& stream, view<C, T>& str);

    // RELATIONAL OPERATORS
    template <typename C, typename T>
    friend bool operator==(const view<C, T>& lhs, const view<C, T>& rhs) noexcept;

    template <typename C, typename T>
    friend bool operator==(const string<C, T>& lhs, const view<C, T>& rhs) noexcept;

    template <typename C, typename T>
    friend bool operator==(const view<C, T>& lhs, const string<C, T>& rhs) noexcept;

    template <typename C, typename T>
    friend bool operator==(const C* lhs, const view<C, T>& rhs);

    template <typename C, typename T>
    friend bool operator==(const view<C, T>& lhs, const C* rhs);

    template <typename C, typename T>
    friend bool operator!=(const view<C, T>& lhs, const view<C, T>& rhs) noexcept;

    template <typename C, typename T>
    friend bool operator!=(const string<C, T>& lhs, const view<C, T>& rhs) noexcept;

    template <typename C, typename T>
    friend bool operator!=(const view<C, T>& lhs, const string<C, T>& rhs) noexcept;

    template <typename C, typename T>
    friend bool operator!=(const C* lhs, const view<C, T>& rhs);

    template <typename C, typename T>
    friend bool operator!=(const view<C, T>& lhs, const C* rhs);

    template <typename C, typename T>
    friend bool operator<(const view<C, T>& lhs, const view<C, T>& rhs) noexcept;

    template <typename C, typename T>
    friend bool operator<(const string<C, T>& lhs, const view<C, T>& rhs) noexcept;

    template <typename C, typename T>
    friend bool operator<(const view<C, T>& lhs, const string<C, T>& rhs) noexcept;

    template <typename C, typename T>
    friend bool operator<(const C* lhs, const view<C, T>& rhs);

    template <typename C, typename T>
    friend bool operator<(const view<C, T>& lhs, const C* rhs);

    template <typename C, typename T>
    friend bool operator<=(const view<C, T>& lhs, const view<C, T>& rhs) noexcept;

    template <typename C, typename T>
    friend bool operator<=(const string<C, T>& lhs, const view<C, T>& rhs) noexcept;

    template <typename C, typename T>
    friend bool operator<=(const view<C, T>& lhs, const string<C, T>& rhs) noexcept;

    template <typename C, typename T>
    friend bool operator<=(const C* lhs, const view<C, T>& rhs);

    template <typename C, typename T>
    friend bool operator<=(const view<C, T>& lhs, const C* rhs);

    template <typename C, typename T>
    friend bool operator>(const view<C, T>& lhs, const view<C, T>& rhs) noexcept;

    template <typename C, typename T>
    friend bool operator>(const string<C, T>& lhs, const view<C, T>& rhs) noexcept;

    template <typename C, typename T>
    friend bool operator>(const view<C, T>& lhs, const string<C, T>& rhs) noexcept;

    template <typename C, typename T>
    friend bool operator>(const C* lhs, const view<C, T>& rhs);

    template <typename C, typename T>
    friend bool operator>(const view<C, T>& lhs, const C* rhs);

    template <typename C, typename T>
    friend bool operator>=(const view<C, T>& lhs, const view<C, T>& rhs) noexcept;

    template <typename C, typename T>
    friend bool operator>=(const string<C, T>& lhs, const view<C, T>& rhs) noexcept;

    template <typename C, typename T>
    friend bool operator>=(const view<C, T>& lhs, const string<C, T>& rhs) noexcept;

    template <typename C, typename T>
    friend bool operator>=(const C* lhs, const view<C, T>& rhs);

    template <typename C, typename T>
    friend bool operator>=(const view<C, T>& lhs, const C* rhs);
};

// IMPLEMENTATION
// --------------

template <typename C, typename T>
const typename basic_string_view<C, T>::size_type basic_string_view<C, T>::npos;

template <typename C, typename T>
void swap(basic_string_view<C, T>& lhs, basic_string_view<C, T>& rhs)
{
    lhs.swap(rhs);
}


template <typename C, typename T>
std::basic_istream<C, T> & operator>>(std::basic_istream<C, T> &stream, basic_string_view<C, T>& str)
{
    return stream.read(const_cast<char*>(str.data_), str.length_);
}


template <typename C, typename T>
std::basic_ostream<C, T> & operator<<(std::basic_ostream<C, T> &stream, basic_string_view<C, T>& str)
{
    return stream.write(str.data(), str.length());
}


template <typename C, typename T>
bool operator==(const basic_string_view<C, T>& lhs, const basic_string_view<C, T>& rhs) noexcept
{
    size_t lhs_size = lhs.size();
    return lhs_size == rhs.size() && T::compare(lhs.data(), rhs.data(), lhs_size) == 0;
}


template <typename C, typename T>
bool operator==(const std::basic_string<C, T>& lhs, const basic_string_view<C, T>& rhs) noexcept
{
    return basic_string_view<C, T>(lhs) == rhs;
}


template <typename C, typename T>
bool operator==(const basic_string_view<C, T>& lhs, const std::basic_string<C, T>& rhs) noexcept
{
    return lhs == basic_string_view<C, T>(rhs);
}


template <typename C, typename T>
bool operator==(const C* lhs, const basic_string_view<C, T>& rhs)
{
    return basic_string_view<C, T>(lhs) == rhs;
}


template <typename C, typename T>
bool operator==(const basic_string_view<C, T>& lhs, const C* rhs)
{
    return lhs == basic_string_view<C, T>(rhs);
}


template <typename C, typename T>
bool operator!=(const basic_string_view<C, T>& lhs, const basic_string_view<C, T>& rhs) noexcept
{

    return !(lhs == rhs);
}


template <typename C, typename T>
bool operator!=(const std::basic_string<C, T>& lhs, const basic_string_view<C, T>& rhs) noexcept
{
    return !(lhs == rhs);
}


template <typename C, typename T>
bool operator!=(const basic_string_view<C, T>& lhs,
    const std::basic_string<C, T>& rhs) noexcept
{
    return !(lhs == rhs);
}


template <typename C, typename T>
bool operator!=(const C* lhs, const basic_string_view<C, T>& rhs)
{
    return !(lhs == rhs);
}


template <typename C, typename T>
bool operator!=(const basic_string_view<C, T>& lhs, const C* rhs)
{
    return !(lhs == rhs);
}


template <typename C, typename T>
bool operator<(const basic_string_view<C, T>& lhs, const basic_string_view<C, T>& rhs) noexcept
{
    return lhs.compare(rhs) < 0;
}


template <typename C, typename T>
bool operator<(const std::basic_string<C, T>& lhs, const basic_string_view<C, T>& rhs) noexcept
{
    return basic_string_view<C, T>(lhs) < rhs;
}


template <typename C, typename T>
bool operator<(const basic_string_view<C, T>& lhs, const std::basic_string<C, T>& rhs) noexcept
{
    return lhs < basic_string_view<C, T>(rhs);
}


template <typename C, typename T>
bool operator<(const C* lhs, const basic_string_view<C, T>& rhs)
{
    return basic_string_view<C, T>(lhs) < rhs;
}


template <typename C, typename T>
bool operator<(const basic_string_view<C, T>& lhs, const C* rhs)
{
    return lhs < basic_string_view<C, T>(rhs);
}


template <typename C, typename T>
bool operator<=(const basic_string_view<C, T>& lhs, const basic_string_view<C, T>& rhs) noexcept
{
    return !(rhs < lhs);
}


template <typename C, typename T>
bool operator<=(const std::basic_string<C, T>& lhs, const basic_string_view<C, T>& rhs) noexcept
{
    return basic_string_view<C, T>(lhs) <= rhs;
}


template <typename C, typename T>
bool operator<=(const basic_string_view<C, T>& lhs, const std::basic_string<C, T>& rhs) noexcept
{
    return lhs <= basic_string_view<C, T>(rhs);
}


template <typename C, typename T>
bool operator<=(const C* lhs, const basic_string_view<C, T>& rhs)
{
    return basic_string_view<C, T>(lhs) <= rhs;
}


template <typename C, typename T>
bool operator<=(const basic_string_view<C, T>& lhs, const C* rhs)
{
    return lhs <= basic_string_view<C, T>(rhs);
}


template <typename C, typename T>
bool operator>(const basic_string_view<C, T>& lhs, const basic_string_view<C, T>& rhs) noexcept
{
    return rhs < lhs;
}


template <typename C, typename T>
bool operator>(const std::basic_string<C, T>& lhs,
    const basic_string_view<C, T>& rhs) noexcept
{
    return basic_string_view<C, T>(lhs) > rhs;
}


template <typename C, typename T>
bool operator>(const basic_string_view<C, T>& lhs, const std::basic_string<C, T>& rhs) noexcept
{
    return lhs > basic_string_view<C, T>(rhs);
}


template <typename C, typename T>
bool operator>(const C* lhs, const basic_string_view<C, T>& rhs)
{
    return basic_string_view<C, T>(lhs) > rhs;
}


template <typename C, typename T>
bool operator>(const basic_string_view<C, T>& lhs, const C* rhs)
{
    return lhs > basic_string_view<C, T>(rhs);
}


template <typename C, typename T>
bool operator>=(const basic_string_view<C, T>& lhs, const basic_string_view<C, T>& rhs) noexcept
{
    return !(lhs < rhs);
}


template <typename C, typename T>
bool operator>=(const std::basic_string<C, T>& lhs, const basic_string_view<C, T>& rhs) noexcept
{
    return basic_string_view<C, T>(lhs) >= rhs;
}


template <typename C, typename T>
bool operator>=(const basic_string_view<C, T>& lhs, const std::basic_string<C, T>& rhs) noexcept
{
    return lhs >= basic_string_view<C, T>(rhs);
}


template <typename C, typename T>
bool operator>=(const C* lhs, const basic_string_view<C, T>& rhs)
{
    return basic_string_view<C, T>(lhs) >= rhs;
}


template <typename C, typename T>
bool operator>=(const basic_string_view<C, T>& lhs, const C* rhs)
{
    return lhs >= basic_string_view<C, T>(rhs);
}


template <typename C, typename T>
basic_string_view<C, T>::basic_string_view(const self& str):
    data_(str.data()),
    length_(str.length())
{}


template <typename C, typename T>
basic_string_view<C, T>&  basic_string_view<C, T>::operator=(const self& str)
{
    data_ = str.data_;
    length_ = str.length_;
    return *this;
}


template <typename C, typename T>
basic_string_view<C, T>::basic_string_view(self&& str)
{
    swap(str);
}


template <typename C, typename T>
auto basic_string_view<C, T>::operator=(self&& str) -> self&
{
    swap(str);
    return *this;
}


template <typename C, typename T>
basic_string_view<C, T>::basic_string_view(const stl_type& str):
    data_(str.data()),
    length_(str.length())
{}


template <typename C, typename T>
basic_string_view<C, T>::basic_string_view(const self& str, size_type pos, size_type len)
{
    size_type size_ = str.size();
    if (pos > size_) {
        throw std::out_of_range("basic_string_view::basic_string_view().");
    }
    data_ = str.data() + pos;
    length_ = std::min(len, size_ - pos);
}


template <typename C, typename T>
basic_string_view<C, T>::basic_string_view(const stl_type& str, size_type pos, size_type len)
{
    operator=(self(str), pos, len);
}


template <typename C, typename T>
basic_string_view<C, T>::basic_string_view(const C* str)
{
    data_ = str;
    length_ = str ? traits_type::length(str) : 0;
}


template <typename C, typename T>
basic_string_view<C, T>::basic_string_view(const C* str,
    size_t n)
{
    data_ = str;
    length_ = n;
}


template <typename C, typename T>
basic_string_view<C, T>::basic_string_view(const C* begin,
        const C* end):
    data_(begin),
    length_(end - begin)
{}


template <typename C, typename T>
basic_string_view<C, T>&  basic_string_view<C, T>::operator=(const C* str)
{
    assign(str);
    return *this;
}


template <typename C, typename T>
auto basic_string_view<C, T>::begin() -> iterator
{
    return const_cast<iterator>(data_);
}


template <typename C, typename T>
auto basic_string_view<C, T>::end() -> iterator
{
    return begin() + length_;
}


template <typename C, typename T>
auto basic_string_view<C, T>::begin() const -> const_iterator
{
    return data_;
}


template <typename C, typename T>
auto basic_string_view<C, T>::end() const -> const_iterator
{
    return data_ + length_;
}


template <typename C, typename T>
auto basic_string_view<C, T>::rbegin() const -> const_reverse_iterator
{
    return const_reverse_iterator(end());
}


template <typename C, typename T>
auto basic_string_view<C, T>::rend() const -> const_reverse_iterator
{
    return const_reverse_iterator(begin());
}


template <typename C, typename T>
auto basic_string_view<C, T>::cbegin() const -> const_iterator
{
    return begin();
}


template <typename C, typename T>
auto basic_string_view<C, T>::cend() const -> const_iterator
{
    return end();
}


template <typename C, typename T>
auto basic_string_view<C, T>::crbegin() const -> const_reverse_iterator
{
    return rbegin();
}


template <typename C, typename T>
auto basic_string_view<C, T>::crend() const -> const_reverse_iterator
{
    return rend();
}


template <typename C, typename T>
auto basic_string_view<C, T>::size() const -> size_type
{
    return length_;
}


template <typename C, typename T>
auto basic_string_view<C, T>::length() const -> size_type
{
    return length_;
}


template <typename C, typename T>
bool basic_string_view<C, T>::empty() const noexcept
{
    return length_ == 0;
}


template <typename C, typename T>
auto basic_string_view<C, T>::operator[](size_type pos) -> reference
{
    assert(pos <= size() && "string index out of bounds");
    return *const_cast<char*>(data_ + pos);
}


template <typename C, typename T>
auto basic_string_view<C, T>::operator[](size_type pos) const -> const_reference
{
    assert(pos <= size() && "string index out of bounds");
    return *(data_ + pos);
}


template <typename C, typename T>
auto basic_string_view<C, T>::at(size_type pos) -> reference
{
    return operator[](pos);
}


template <typename C, typename T>
auto basic_string_view<C, T>::at(size_type pos) const -> const_reference
{
    return operator[](pos);
}


template <typename C, typename T>
auto basic_string_view<C, T>::front() -> reference
{
    assert(!empty() && "string::front(): string is empty");
    return *const_cast<char*>(data_);
}


template <typename C, typename T>
auto basic_string_view<C, T>::front() const -> const_reference
{
    assert(!empty() && "string::front(): string is empty");
    return *data_;
}


template <typename C, typename T>
auto basic_string_view<C, T>::back() -> reference
{
    assert(!empty() && "string::back(): string is empty");
    return *const_cast<char*>(data_ + length_ - 1);
}


template <typename C, typename T>
auto basic_string_view<C, T>::back() const -> const_reference
{
    assert(!empty() && "string::back(): string is empty");
    return *(data_ + length_ - 1);
}


template <typename C, typename T>
basic_string_view<C, T>&  basic_string_view<C, T>::assign(const self& str)
{
    data_ = str.data();
    length_ = str.length();
    return *this;
}


template <typename C, typename T>
basic_string_view<C, T>&  basic_string_view<C, T>::assign(const stl_type& str)
{
    return assign(basic_string_view<C, T>(str));
}


template <typename C, typename T>
auto basic_string_view<C, T>::assign(const self& str, size_type subpos, size_type sublen) -> self&
{
    size_type size = str.size();
    if (subpos > size) {
        throw std::out_of_range("basic_string_view::assign().");
    }
    return assign(str.data() + subpos, std::min(sublen, size - subpos));
}


template <typename C, typename T>
auto basic_string_view<C, T>::assign(const stl_type& str, size_type subpos, size_type sublen) -> self&
{
    return assign(basic_string_view<C, T>(str), subpos, sublen);
}


template <typename C, typename T>
auto basic_string_view<C, T>::assign(const_pointer s) -> self&
{
    data_ = s;
    length_ = s ? traits_type::length(s) : 0;
    return *this;
}


template <typename C, typename T>
auto basic_string_view<C, T>::assign(const_pointer s, size_type n) -> self&
{
    data_ = s;
    length_ = n;
    return *this;
}


template <typename C, typename T>
void basic_string_view<C, T>::swap(basic_string_view<C, T>& other)
{
    std::swap(data_, other.data_);
    std::swap(length_, other.length_);
}


template <typename C, typename T>
auto basic_string_view<C, T>::operator++() -> self&
{
    if (length_) {
        ++data_;
        --length_;
    }
    if (empty()) {
        data_ = nullptr;
    }

    return *this;
}


template <typename C, typename T>
basic_string_view<C, T> basic_string_view<C, T>::operator++(int)
{
    self copy(*this);
    operator++();

    return copy;
}


template <typename C, typename T>
basic_string_view<C, T>&  basic_string_view<C, T>::operator--()
{
    if (!empty()) {
        --data_;
        ++length_;
    }
    return *this;
}


template <typename C, typename T>
auto basic_string_view<C, T>::operator--(int) -> self
{
    self copy(*this);
    operator--();

    return copy;
}


template <typename C, typename T>
auto basic_string_view<C, T>::operator+=(size_type shift) -> self&
{
    size_type offset = std::min<size_type>(shift, length_);
    data_ += offset;
    length_ -= offset;

    return *this;
}


template <typename C, typename T>
auto basic_string_view<C, T>::operator+(size_type shift) -> self
{
    self copy(*this);
    copy += shift;
    return copy;
}


template <typename C, typename T>
auto basic_string_view<C, T>::operator-=(size_type shift) -> self&
{
    if (!empty()) {
        data_ -= shift;
        length_ += shift;
    }
    return *this;
}


template <typename C, typename T>
auto basic_string_view<C, T>::operator-(size_type shift) -> self
{
    self copy(*this);
    copy -= shift;
    return copy;
}


template <typename C, typename T>
auto basic_string_view<C, T>::c_str() const noexcept -> const_pointer
{
    return data_;
}


template <typename C, typename T>
auto basic_string_view<C, T>::data() const noexcept -> const_pointer
{
    return data_;
}


template <typename C, typename T>
auto basic_string_view<C, T>::find(const self& str, size_type pos) const noexcept -> size_type
{
    auto found = ::find(data()+pos, size()-pos, str.data(), str.size());
    return found ? found - data() : npos;
}


template <typename C, typename T>
auto basic_string_view<C, T>::find(const stl_type& str, size_type pos) const -> size_type
{
    const_pointer first = str.data();
    const size_t length = str.size();
    auto found = ::find(data()+pos, size()-pos, first, length);
    return found ? found - data() : npos;
}


template <typename C, typename T>
auto basic_string_view<C, T>::find(const_pointer array, size_type pos) const -> size_type
{
    const_pointer first = array;
    size_type length = traits_type::length(array);
    auto found = ::find(data()+pos, size()-pos, first, length);
    return found ? found - data() : npos;
}


template <typename C, typename T>
auto basic_string_view<C, T>::find(const_pointer array, size_type pos, size_type length) const -> size_type
{
    auto found = ::find(data()+pos, size()-pos, array, length);
    return found ? found - data() : npos;
}


template <typename C, typename T>
auto basic_string_view<C, T>::find(value_type c, size_type pos) const noexcept -> size_type
{
    auto found = ::find(data()+pos, size()-pos, &c, 1);
    return found ? found - data() : npos;
}


template <typename C, typename T>
auto basic_string_view<C, T>::find_first_of(const self& str, size_type pos) const noexcept -> size_type
{
    auto found = ::find_of(data()+pos, size()-pos, str.data(), str.size());
    return found ? found - data() : npos;
}


template <typename C, typename T>
auto basic_string_view<C, T>::find_first_of(const stl_type &str, size_type pos) const -> size_type
{
    const_pointer first = str.data();
    size_type length = str.size();
    auto found = ::find_of(data()+pos, size()-pos, first, length);
    return found ? found - data() : npos;
}


template <typename C, typename T>
auto basic_string_view<C, T>::find_first_of(const_pointer array, size_type pos) const -> size_type
{
    const_pointer first = array;
    size_type length = traits_type::length(array);
    auto found = ::find_of(data()+pos, size()-pos, first, length);
    return found ? found - data() : npos;
}


template <typename C, typename T>
auto basic_string_view<C, T>::find_first_of(const_pointer array, size_type pos, size_type length) const -> size_type
{
    auto found = ::find_of(data()+pos, size()-pos, array, length);
    return found ? found - data() : npos;
}


template <typename C, typename T>
auto basic_string_view<C, T>::find_first_of(value_type c, size_type pos) const noexcept -> size_type
{
    auto found = ::find_of(data()+pos, size()-pos, &c, 1);
    return found ? found - data() : npos;
}


template <typename C, typename T>
auto basic_string_view<C, T>::find_first_not_of(const self& str, size_type pos) const noexcept -> size_type
{
    auto found = ::find_not_of(data()+pos, size()-pos, str.data(), str.size());
    return found ? found - data() : npos;
}


template <typename C, typename T>
auto basic_string_view<C, T>::find_first_not_of(const stl_type &str, size_type pos) const -> size_type
{
    const_pointer first = str.data();
    size_type length = str.size();
    auto found = ::find_not_of(data()+pos, size()-pos, first, length);
    return found ? found - data() : npos;
}


template <typename C, typename T>
auto basic_string_view<C, T>::find_first_not_of(const_pointer array, size_type pos) const -> size_type
{
    const_pointer first = array;
    size_type length = traits_type::length(array);
    auto found = ::find_not_of(data()+pos, size()-pos, first, length);
    return found ? found - data() : npos;
}


template <typename C, typename T>
auto basic_string_view<C, T>::find_first_not_of(const_pointer array, size_type pos, size_type length) const -> size_type
{
    auto found = ::find_not_of(data()+pos, size()-pos, array, length);
    return found ? found - data() : npos;
}


template <typename C, typename T>
auto basic_string_view<C, T>::find_first_not_of(value_type c, size_type pos) const noexcept -> size_type
{
    auto found = ::find_not_of(data()+pos, size()-pos, &c, 1);
    return found ? found - data() : npos;
}


template <typename C, typename T>
auto basic_string_view<C, T>::rfind(const self& str, size_type pos) const noexcept -> size_type
{
    auto found = ::rfind(end(), size()-pos, str.data(), str.size());
    return found ? found - data() : npos;
}


template <typename C, typename T>
auto basic_string_view<C, T>::rfind(const stl_type &str, size_type pos) const -> size_type
{
    const_pointer first = str.data();
    size_type length = str.size();
    auto found = ::rfind(end(), size()-pos, first, length);
    return found ? found - data() : npos;
}


template <typename C, typename T>
auto basic_string_view<C, T>::rfind(const_pointer array, size_type pos) const -> size_type
{
    const_pointer first = array;
    size_type length = traits_type::length(array);
    auto found = ::rfind(end(), size()-pos, first, length);
    return found ? found - data() : npos;
}


template <typename C, typename T>
auto basic_string_view<C, T>::rfind(const_pointer array, size_type pos, size_type length) const -> size_type
{
    auto found = ::rfind(end(), size()-pos, array, length);
    return found ? found - data() : npos;
}


template <typename C, typename T>
auto basic_string_view<C, T>::rfind(value_type c, size_type pos) const noexcept -> size_type
{
    auto found = ::rfind(end(), size()-pos, &c, 1);
    return found ? found - data() : npos;
}


template <typename C, typename T>
auto basic_string_view<C, T>::find_last_of(const self& str, size_type pos) const noexcept -> size_type
{
    auto found = ::rfind_of(end(), size()-pos, str.data(), str.size());
    return found ? found - data() : npos;
}


template <typename C, typename T>
auto basic_string_view<C, T>::find_last_of(const stl_type &str, size_type pos) const -> size_type
{
    const_pointer first = str.data();
    size_type length = str.size();
    auto found = ::rfind_of(end(), size()-pos, first, length);
    return found ? found - data() : npos;
}


template <typename C, typename T>
auto basic_string_view<C, T>::find_last_of(const_pointer array, size_type pos) const -> size_type
{
    const_pointer first = array;
    size_type length = traits_type::length(array);
    auto found = ::rfind_of(end(), size()-pos, first, length);
    return found ? found - data() : npos;
}


template <typename C, typename T>
auto basic_string_view<C, T>::find_last_of(const_pointer array, size_type pos, size_type length) const -> size_type
{
    auto found = ::rfind_of(end(), size()-pos, array, length);
    return found ? found - data() : npos;
}


template <typename C, typename T>
auto basic_string_view<C, T>::find_last_of(value_type c, size_type pos) const noexcept -> size_type
{
    auto found = ::rfind_of(end(), size()-pos, &c, 1);
    return found ? found - data() : npos;
}


template <typename C, typename T>
auto basic_string_view<C, T>::find_last_not_of(const self& str, size_type pos) const noexcept -> size_type
{
    auto found = ::rfind_not_of(end(), size()-pos, str.data(), str.size());
    return found ? found - data() : npos;
}


template <typename C, typename T>
auto basic_string_view<C, T>::find_last_not_of(const stl_type &str, size_type pos) const -> size_type
{
    const_pointer first = str.data();
    size_type length = str.size();
    auto found = ::rfind_not_of(end(), size()-pos, first, length);
    return found ? found - data() : npos;
}


template <typename C, typename T>
auto basic_string_view<C, T>::find_last_not_of(const_pointer array, size_type pos) const -> size_type
{
    const_pointer first = array;
    size_type length = traits_type::length(array);
    auto found = ::rfind_not_of(end(), size()-pos, first, length);
    return found ? found - data() : npos;
}


template <typename C, typename T>
auto basic_string_view<C, T>::find_last_not_of(const_pointer array, size_type pos, size_type length) const -> size_type
{
    auto found = ::rfind_not_of(end(), size()-pos, array, length);
    return found ? found - data() : npos;
}


template <typename C, typename T>
auto basic_string_view<C, T>::find_last_not_of(value_type c, size_type pos) const noexcept -> size_type
{
    auto found = ::rfind_not_of(end(), size()-pos, &c, 1);
    return found ? found - data() : npos;
}


template <typename C, typename T>
int basic_string_view<C, T>::compare(const self& str) const noexcept
{
    size_type lhs_size = size();
    size_type rhs_size = str.size();
    int result = traits_type::compare(data(), str.data(), std::min(lhs_size, rhs_size));
    if (result != 0) {
        return result;
    } else if (lhs_size < rhs_size) {
        return -1;
    } else if (lhs_size > rhs_size) {
        return 1;
    }
    return 0;
}


template <typename C, typename T>
int basic_string_view<C, T>::compare(const stl_type& str) const noexcept
{
    return compare(self(str));
}


template <typename C, typename T>
int basic_string_view<C, T>::compare(size_type pos, size_type len, const self& str) const
{
    return self(*this, pos, len).compare(str);
}


template <typename C, typename T>
int basic_string_view<C, T>::compare(size_type pos, size_type len, const stl_type& str) const
{
    return self(*this, pos, len).compare(str);
}


template <typename C, typename T>
int basic_string_view<C, T>::compare(size_type pos, size_type len,
    const self& str,
    size_type subpos,
    size_type sublen) const
{
    return self(*this, pos, len).compare(self(str, subpos, sublen));
}


template <typename C, typename T>
int basic_string_view<C, T>::compare(size_type pos, size_type len, const stl_type& str, size_type subpos, size_type sublen) const
{
    return self(*this, pos, len).compare(self(str, subpos, sublen));
}


template <typename C, typename T>
int basic_string_view<C, T>::compare(const_pointer s) const
{
    return compare(self(s));
}


template <typename C, typename T>
int basic_string_view<C, T>::compare(size_type pos, size_type len, const_pointer s) const
{
    return self(*this, pos, len).compare(s);
}


template <typename C, typename T>
int basic_string_view<C, T>::compare(size_type pos, size_type len, const_pointer s, size_type n) const
{
    return self(*this, pos, len).compare(s, n);
}


template <typename C, typename T>
auto basic_string_view<C, T>::substr(size_type pos, size_type len) const -> self
{
    return self(*this, pos, len);
}


template <typename C, typename T>
basic_string_view<C, T>::operator bool() const
{
    return !empty();
}


template <typename C, typename T>
basic_string_view<C, T>::operator stl_type() const
{
    return stl_type(data_, length_);
}

// TYPES
// -----

typedef basic_string_view<char> string_view;
typedef basic_string_view<wchar_t> wstring_view;
typedef basic_string_view<char16_t> u16string_view;
typedef basic_string_view<char32_t> u32string_view;

PYCPP_END_NAMESPACE
