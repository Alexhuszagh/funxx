//  :copyright: (c) 2017 Alex Huszagh.
//  :license: MIT, see licenses/mit.md for more details.
/**
 *  \addtogroup PyCPP
 *  \brief Iterator utilities.
 */

#pragma once

#include <pycpp/config.h>
#include <pycpp/preprocessor/compiler.h>
#include <iterator>

PYCPP_BEGIN_NAMESPACE

// DECLARATION
// -----------

template <typename T>
using is_input_iterator = std::is_same<typename std::iterator_traits<T>::iterator_category, std::input_iterator_tag>;

template <typename T>
using is_output_iterator = std::is_same<typename std::iterator_traits<T>::iterator_category, std::output_iterator_tag>;

template <typename T>
using is_forward_iterator = std::is_same<typename std::iterator_traits<T>::iterator_category, std::forward_iterator_tag>;

template <typename T>
using is_bidirectional_iterator = std::is_same<typename std::iterator_traits<T>::iterator_category, std::bidirectional_iterator_tag>;

template <typename T>
using is_random_access_iterator = std::is_same<typename std::iterator_traits<T>::iterator_category, std::random_access_iterator_tag>;

#ifdef HAVE_CPP14

// SFINAE
// ------

template <typename T>
constexpr bool is_input_iterator_v = is_input_iterator<T>::value;

template <typename T>
constexpr bool is_output_iterator_v = is_output_iterator<T>::value;

template <typename T>
constexpr bool is_forward_iterator_v = is_forward_iterator<T>::value;

template <typename T>
constexpr bool is_bidirectional_iterator_v = is_bidirectional_iterator<T>::value;

template <typename T>
constexpr bool is_random_access_iterator_v = is_random_access_iterator<T>::value;

#endif

PYCPP_END_NAMESPACE
