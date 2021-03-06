//  :copyright: (c) 2016 Denilson das Mercês Amorim.
//  :copyright: (c) 2017 Alex Huszagh.
//  :license: Boost, see licenses/boost.md for more details.
/**
 *  \addtogroup PyCPP
 *  \brief Any implementation for C++11.
 *
 *  For GCC and Clang, you may use the C++17 syntax with the `any`
 *  implementation from C++11 onwards, for MSVC, at least until C++17,
 *  you must explicitly specify the type overloads to `make_any`.
 *  For example:
 *
 *  \code
 *      any p0 = 1;                     // works on all but MSVC
 *      any p1 = make_any<int>(1);      // works on all compilers
 *
 *  \synopsis
 */

#pragma once

#include <pycpp/config.h>
#include <pycpp/preprocessor/compiler.h>

#if defined(HAVE_CPP17)             // HAVE_CPP17
#   include <any>
#else                               // !HAVE_CPP17
#   include <pycpp/stl/initializer_list.h>
#   include <pycpp/stl/memory.h>
#   include <pycpp/stl/stdexcept.h>
#   include <pycpp/stl/typeinfo.h>
#   include <pycpp/stl/type_traits.h>
#   include <pycpp/stl/utility.h>
#endif                              // HAVE_CPP17

PYCPP_BEGIN_NAMESPACE

#if defined(HAVE_CPP17)             // HAVE_CPP17

// ALIAS
// -----

using std::any;
using std::bad_any_cast;
using std::any_cast;
using std::make_any;

#else                               // !HAVE_CPP17

namespace detail
{
// DETAIL
// ------

/**
 *  \brief 2 words-based storage for union.
 */
union storage_union
{
    using stack_storage_t = aligned_storage_t<2 * sizeof(void*), alignment_of<void*>::value>;

    void* dynamic;
    stack_storage_t stack;
};

/**
 *  \brief Base VTable specification.
 */
struct vtable_type
{
    // Note: The caller is responsible for doing .vtable = nullptr after destructful operations
    // such as destroy() and/or move().

    /// The type of the object this vtable is for.
    const type_info& (*type)() noexcept;

    /// Destroys the object in the union.
    /// The state of the union after this call is unspecified, caller must ensure not to use src anymore.
    void(*destroy)(storage_union&) noexcept;

    /// Copies the **inner** content of the src union into the yet unitialized dest union.
    /// As such, both inner objects will have the same state, but on separate memory locations.
    void(*copy)(const storage_union& src, storage_union& dest);

    /// Moves the storage from src to the yet unitialized dest union.
    /// The state of src after this call is unspecified, caller must ensure not to use src anymore.
    void(*move)(storage_union& src, storage_union& dest) noexcept;

    /// Exchanges the storage between lhs and rhs.
    void(*swap)(storage_union& lhs, storage_union& rhs) noexcept;
};


/**
 *  \brief VTable for dynamically allocated storage.
 */
template <typename T>
struct vtable_dynamic
{
    static const type_info& type() noexcept
    {
        return typeid(T);
    }

    static void destroy(storage_union& storage) noexcept
    {
        delete reinterpret_cast<T*>(storage.dynamic);
    }

    static void copy(const storage_union& src, storage_union& dest)
    {
        dest.dynamic = new T(*reinterpret_cast<const T*>(src.dynamic));
    }

    static void move(storage_union& src, storage_union& dest) noexcept
    {
        dest.dynamic = src.dynamic;
        src.dynamic = nullptr;
    }

    static void swap(storage_union& lhs, storage_union& rhs) noexcept
    {
        using PYCPP_NAMESPACE::swap;
        swap(lhs.dynamic, rhs.dynamic);
    }
};

/**
 *  \brief VTable for stack allocated storage.
 */
template <typename T>
struct vtable_stack
{
    static const type_info& type() noexcept
    {
        return typeid(T);
    }

    static void destroy(storage_union& storage) noexcept
    {
        reinterpret_cast<T*>(&storage.stack)->~T();
    }

    static void copy(const storage_union& src, storage_union& dest)
    {
        new (&dest.stack) T(reinterpret_cast<const T&>(src.stack));
    }

    static void move(storage_union& src, storage_union& dest) noexcept
    {
        // one of the conditions for using vtable_stack is a nothrow move constructor,
        // so this move constructor will never throw a exception.
        new (&dest.stack) T(PYCPP_NAMESPACE::move(reinterpret_cast<T&>(src.stack)));
        destroy(src);
    }

    static void swap(storage_union& lhs, storage_union& rhs) noexcept
    {
        using PYCPP_NAMESPACE::swap;
        swap(reinterpret_cast<T&>(lhs.stack), reinterpret_cast<T&>(rhs.stack));
    }
};

/**
 *  Whether the type T must be dynamically allocated or can be stored on the stack.
 */
template <typename T>
struct requires_allocation :
    bool_constant<
        !(is_nothrow_move_constructible<T>::value &&    // N4562 §6.3/3 [any.class]
          sizeof(T) <= sizeof(storage_union::stack) &&
          alignment_of<T>::value <= alignment_of<storage_union::stack_storage_t>::value)>
{};

/**
 *  Returns the pointer to the vtable of the type T.
 */
template <typename T>
static vtable_type* vtable_for_type()
{
    using type = conditional_t<requires_allocation<T>::value, vtable_dynamic<T>, vtable_stack<T>>;
    static vtable_type table = {
        type::type, type::destroy,
        type::copy, type::move,
        type::swap,
    };
    return &table;
}

template <typename V>
inline V any_cast_move_if_true(remove_reference_t<V>* p, true_type)
{
    return move(*p);
}

template <typename V>
inline V any_cast_move_if_true(remove_reference_t<V>* p, false_type)
{
    return *p;
}

}   /* detail */

// OBJECTS
// -------

struct bad_any_cast: public bad_cast
{
    virtual const char* what() const noexcept override;
};

struct any
{
public:
    any() noexcept;
    any(const any&);
    any& operator=(const any&);
    any(any&&) noexcept;
    any& operator=(any&&) noexcept;
    ~any();

    template<typename V, typename = enable_if_t<!is_same<decay_t<V>, any>::value>>
    any(V&& value)
    {
        static_assert(is_copy_constructible<decay_t<V>>::value,
            "T shall satisfy the CopyConstructible requirements.");
        construct(forward<V>(value));
    }

    template<typename V, typename = enable_if_t<!is_same<decay_t<V>, any>::value>>
    any& operator=(V&& value)
    {
        static_assert(is_copy_constructible<decay_t<V>>::value,
            "T shall satisfy the CopyConstructible requirements.");
        any(forward<V>(value)).swap(*this);
        return *this;
    }

    template <typename V, typename... Ts>
    explicit any(in_place_type_t<V>, Ts&&... ts)
    {
        construct(V(forward<Ts>(ts)...));
    }

    template <typename V, typename U, typename... Ts>
    explicit any(in_place_type_t<V>, initializer_list<U> list, Ts&&... ts)
    {
        construct(V(move(list), forward<Ts>(ts)...));
    }

    // MODIFIERS
    template <
        typename V,
        typename... Ts,
        typename T = decay_t<V>,
        typename = enable_if_t<
            is_constructible<T, Ts...>::value &&
            is_copy_constructible<T>::value
        >
    >
    T& emplace(Ts&&... ts)
    {
        reset();
        return construct(forward<V>(ts...));
    }

    template <
        typename V,
        typename U,
        typename... Ts,
        typename T = decay_t<V>,
        typename = enable_if_t<
            is_constructible<T, initializer_list<U>&, Ts...>::value &&
            is_copy_constructible<T>::value
        >
    >
    T& emplace(initializer_list<U> list, Ts&&... ts)
    {
        reset();
        return construct(forward<V>(list, ts...));
    }

    void reset() noexcept;
    void swap(any&) noexcept;

    // OBSERVERS
    bool has_value() const noexcept;
    const type_info& type() const noexcept;

protected:
    detail::storage_union storage;
    detail::vtable_type* vtable;

    template <typename T>
    friend const T* any_cast(const any*) noexcept;

    template <typename T>
    friend T* any_cast(any*) noexcept;

    bool is_typed(const type_info& t) const;
    static bool is_same(const type_info&, const type_info&);

    template <typename T>
    const T* cast() const noexcept
    {
        return detail::requires_allocation<decay_t<T>>::value?
            reinterpret_cast<const T*>(storage.dynamic) :
            reinterpret_cast<const T*>(&storage.stack);
    }

    template <typename T>
    T* cast() noexcept
    {
        return detail::requires_allocation<decay_t<T>>::value?
            reinterpret_cast<T*>(storage.dynamic) :
            reinterpret_cast<T*>(&storage.stack);
    }

    template <typename V, typename T>
    enable_if_t<detail::requires_allocation<T>::value, T&>
    do_construct(V&& value)
    {
        storage.dynamic = new T(forward<V>(value));
        return *reinterpret_cast<T*>(storage.dynamic);
    }

    template <typename V, typename T>
    enable_if_t<!detail::requires_allocation<T>::value, T&>
    do_construct(V&& value)
    {
        new (&storage.stack) T(forward<V>(value));
        return reinterpret_cast<T&>(storage.stack);
    }

    /**
     *  Chooses between stack and dynamic allocation for the type decay_t<ValueType>,
     *  assigns the correct vtable, and constructs the object on our storage.
     */
    template <typename V, typename T = decay_t<V>>
    T& construct(V&& value)
    {
        this->vtable = detail::vtable_for_type<T>();
        return do_construct<V,T>(forward<V>(value));
    }
};


// FUNCTIONS
// ---------


template <typename V>
inline V any_cast(const any& operand)
{
    auto p = any_cast<add_const_t<remove_reference_t<V>>>(&operand);
    if(p == nullptr) {
        throw bad_any_cast();
    }
    return *p;
}


template <typename V>
inline V any_cast(any& operand)
{
    auto p = any_cast<remove_reference_t<V>>(&operand);
    if(p == nullptr) {
        throw bad_any_cast();
    }
    return *p;
}


template <typename V>
inline V any_cast(any&& operand)
{
    using can_move = bool_constant<
        is_move_constructible<V>::value
        && !is_lvalue_reference<V>::value
    >;

    auto p = any_cast<remove_reference_t<V>>(&operand);
    if(p == nullptr) {
        throw bad_any_cast();
    }
    return detail::any_cast_move_if_true<V>(p, can_move());
}


template <typename T>
inline const T* any_cast(const any* operand) noexcept
{
    if(operand == nullptr || !operand->is_typed(typeid(T))) {
        return nullptr;
    } else {
        return operand->cast<T>();
    }
}


template <typename T>
inline T* any_cast(any* operand) noexcept
{
    if(operand == nullptr || !operand->is_typed(typeid(T))) {
        return nullptr;
    } else {
        return operand->cast<T>();
    }
}


template <typename T, typename... Ts>
any make_any(Ts&&... ts)
{
    return any(in_place_type_t<T>(), forward<Ts>(ts)...);
}


template <typename T, typename U, typename ... Ts>
any make_any(initializer_list<U> list, Ts&&... ts)
{
    return any(in_place_type_t<T>(), list, forward<Ts>(ts)...);
}

#endif                              // HAVE_CPP17

PYCPP_END_NAMESPACE
