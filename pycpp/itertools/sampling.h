//  :copyright: (c) Howard Hinnant 2005-2011.
//  :copyright: (c) Alex Huszagh 2017.
//  :license: Boost,see licenses/boost.md for more details.
/**
 *  \addtogroup PyCPP
 *  \brief Combination and permutation algorithms.
 *
 *  High-performance permutation and combination algorithms
 *  from iterator pairs.
 *
 *  \synopsis
 *      template <class BidirIter, class Function>
 *      Function for_each_permutation(BidirIter first,
 *          BidirIter mid,
 *          BidirIter last,
 *          Function f);
 *
 *      template <class BidirIter, class Function>
 *      Function for_each_reversible_permutation(BidirIter first,
 *          BidirIter mid,
 *          BidirIter last,
 *          Function f);
 *
 *      template <class BidirIter, class Function>
 *      Function for_each_circular_permutation(BidirIter first,
 *          BidirIter mid,
 *          BidirIter last,
 *          Function f);
 *
 *      template <class BidirIter, class Function>
 *      Function for_each_reversible_circular_permutation(BidirIter first,
 *          BidirIter mid,
 *          BidirIter last,
 *          Function f);
 *
 *      template <class BidirIter, class Function>
 *      Function for_each_combination(BidirIter first,
 *          BidirIter mid,
 *          BidirIter last,
 *          Function f);
 *
 *      template <class UInt>
 *      UInt count_each_combination(UInt d1, UInt d2);
 *
 *      template <class UInt>
 *      UInt count_each_permutation(UInt d1, UInt d2);
 *
 *      template <class UInt>
 *      UInt count_each_circular_permutation(UInt d1, UInt d2);
 *
 *      template <class UInt>
 *      UInt count_each_reversible_permutation(UInt d1, UInt d2);
 *
 *      template <class UInt>
 *      UInt count_each_reversible_circular_permutation(UInt d1, UInt d2);
 *
 *      template <class BidirIter>
 *      uintmax_t count_each_combination(BidirIter first,
 *          BidirIter mid,
 *          BidirIter last);
 *
 *      template <class BidirIter>
 *      uintmax_t count_each_permutation(BidirIter first,
 *          BidirIter mid,
 *          BidirIter last);
 *
 *      template <class BidirIter>
 *      uintmax_t count_each_circular_permutation(BidirIter first,
 *          BidirIter mid,
 *          BidirIter last);
 *
 *      template <class BidirIter>
 *      uintmax_t count_each_reversible_permutation(BidirIter first,
 *          BidirIter mid,
 *          BidirIter last);
 *
 *      template <class BidirIter>
 *      uintmax_t count_each_reversible_circular_permutation(BidirIter first,
 *          BidirIter mid,
 *          BidirIter last);
 */

#pragma once

#include <pycpp/stl/algorithm.h>
#include <pycpp/stl/iterator.h>
#include <pycpp/stl/limits.h>
#include <pycpp/stl/stdexcept.h>

PYCPP_BEGIN_NAMESPACE

// FUNCTIONS
// ---------

/**
 *  \brief Rotates two discontinuous ranges to put *first2 where *first1 is.
 *
 *  If last1 == first2 this would be equivalent to rotate(first1, first2,
 *  last2), but instead the rotate "jumps" over the discontinuity [last1,
 *  first2) - which need not be a valid range. In order to make it faster,
 *  the length of [first1, last1) is passed in as d1, and d2 must be the
 *  length of [first2, last2).
 *
 *  In a perfect world the d1 > d2 case would have used swap_ranges and
 *  reverse_iterator, but reverse_iterator is too inefficient.
 */
template <typename BidirIter>
void rotate_discontinuous(BidirIter first1,
    BidirIter last1,
    typename iterator_traits<BidirIter>::difference_type d1,
    BidirIter first2,
    BidirIter last2,
    typename iterator_traits<BidirIter>::difference_type d2)
{
    if (d1 <= d2) {
        rotate(first2, swap_ranges(first1, last1, first2), last2);
    } else {
        BidirIter i1 = last1;
        while (first2 != last2) {
            swap(*--i1, *--last2);
        }
        rotate(first1, i1, last1);
    }
}


/**
 *  \brief Rotates three discontinuous ranges to put *first2 where *first1 is.
 *
 *  Just like rotate_discontinuous, except the second range is now
 *  represented by two discontinuous ranges: [first2, last2) +
 *  [first3, last3).
 */
template <typename BidirIter>
void rotate_discontinuous3(BidirIter first1,
    BidirIter last1,
    typename iterator_traits<BidirIter>::difference_type d1,
    BidirIter first2,
    BidirIter last2,
    typename iterator_traits<BidirIter>::difference_type d2,
    BidirIter first3,
    BidirIter last3,
    typename iterator_traits<BidirIter>::difference_type d3)
{
    rotate_discontinuous(first1, last1, d1, first2, last2, d2);
    if (d1 <= d2) {
        rotate_discontinuous(next(first2, d2 - d1), last2, d1, first3, last3, d3);
    } else {
        rotate_discontinuous(next(first1, d2), last1, d1 - d2, first3, last3, d3);
        rotate_discontinuous(first2, last2, d2, first3, last3, d3);
    }
}


/**
 *  \brief Call f() for each combination of [first1, last1) + [first2, last2).
 *
 *  The items are swapped/rotated into the range [first1, last1). As long
 *  as f() returns false, continue for every combination and then return
 *  [first1, last1) and [first2, last2) to their original state. If f()
 *  returns true, return immediately.
 *
 *  Does the absolute mininum amount of swapping to accomplish its task.
 *  If f() always returns false it will be called (d1+d2)!/(d1!*d2!) times.
 */
template <typename BidirIter, typename Function>
bool combine_discontinuous(BidirIter first1,
    BidirIter last1,
    typename iterator_traits<BidirIter>::difference_type d1,
    BidirIter first2,
    BidirIter last2,
    typename iterator_traits<BidirIter>::difference_type d2,
    Function &f,
    typename iterator_traits<BidirIter>::difference_type d = 0)
{
    using D = typename iterator_traits<BidirIter>::difference_type;
    if (d1 == 0 || d2 == 0) {
        return f();
    } else if (d1 == 1) {
        for (BidirIter i2 = first2; i2 != last2; ++i2) {
            if (f()) {
                return true;
            }
            swap(*first1, *i2);
        }
    } else {
        BidirIter f1p = next(first1);
        BidirIter i2 = first2;
        for (D d22 = d2; i2 != last2; ++i2, --d22) {
            if (combine_discontinuous(f1p, last1, d1-1, i2, last2, d22, f, d+1)) {
                return true;
            }
            swap(*first1, *i2);
        }
    }

    if (f()) {
        return true;
    } else if (d != 0) {
        rotate_discontinuous(first1, last1, d1, next(first2), last2, d2-1);
    } else {
        rotate_discontinuous(first1, last1, d1, first2, last2, d2);
    }
    return false;
}


/**
 *  \brief A binder for binding arguments to call combine_discontinuous.
 */
template <typename Function, typename BidirIter>
class call_combine_discontinuous
{
    using D = typename iterator_traits<BidirIter>::difference_type;
    Function f_;
    BidirIter first1_;
    BidirIter last1_;
    D d1_;
    BidirIter first2_;
    BidirIter last2_;
    D d2_;

public:
    call_combine_discontinuous(BidirIter first1,
            BidirIter last1,
            D d1,
            BidirIter first2,
            BidirIter last2,
            D d2,
            Function &f):
        f_(f),
        first1_(first1),
        last1_(last1),
        d1_(d1),
        first2_(first2),
        last2_(last2),
        d2_(d2)
    {}

    bool operator()()
    {
        return combine_discontinuous(first1_, last1_, d1_, first2_, last2_, d2_, f_);
    }
};


/**
 *  \brief See combine_discontinuous3.
 */
template <typename BidirIter, typename Function>
bool combine_discontinuous3_(
    BidirIter first1,
    BidirIter last1,
    typename iterator_traits<BidirIter>::difference_type d1,
    BidirIter first2,
    BidirIter last2,
    typename iterator_traits<BidirIter>::difference_type d2,
    BidirIter first3,
    BidirIter last3,
    typename iterator_traits<BidirIter>::difference_type d3,
    Function &f,
    typename iterator_traits<BidirIter>::difference_type d = 0)
{
    using D = typename iterator_traits<BidirIter>::difference_type;
    if (d1 == 1) {
        for (BidirIter i2 = first2; i2 != last2; ++i2) {
            if (f()) {
                return true;
            }
            swap(*first1, *i2);
        }
        if (f()) {
            return true;
        }
        swap(*first1, *prev(last2));
        swap(*first1, *first3);
        for (BidirIter i2 = next(first3); i2 != last3; ++i2) {
            if (f()) {
                return true;
            }
            swap(*first1, *i2);
        }
    } else {
        BidirIter f1p = next(first1);
        BidirIter i2 = first2;
        for (D d22 = d2; i2 != last2; ++i2, --d22) {
            if (combine_discontinuous3_(f1p, last1, d1-1, i2, last2, d22, first3, last3, d3, f, d+1)) {
                return true;
            }
            swap(*first1, *i2);
        }
        i2 = first3;
        for (D d22 = d3; i2 != last3; ++i2, --d22) {
            if (combine_discontinuous(f1p, last1, d1-1, i2, last3, d22, f, d+1)) {
                return true;
            }
            swap(*first1, *i2);
        }
    }

    if (f()) {
        return true;
    }
    if (d1 == 1) {
        swap(*prev(last2), *first3);
    }
    if (d != 0) {
        if (d2 > 1) {
            rotate_discontinuous3(first1, last1, d1, next(first2), last2, d2-1, first3, last3, d3);
        } else {
            rotate_discontinuous(first1, last1, d1, first3, last3, d3);
        }
    } else {
        rotate_discontinuous3(first1, last1, d1, first2, last2, d2, first3, last3, d3);
    }
    return false;
}


/**
 *  \brief Like combine_discontinuous
 *
 *  Swaps/rotates each combination out of  [first1, last1) + [first2,
 *  last2) + [first3, last3) into [first1, last1). If f() always returns
 *  false, it is called (d1+d2+d3)!/(d1!*(d2+d3)!) times.
 */
template <typename BidirIter, typename Function>
bool combine_discontinuous3(BidirIter first1,
    BidirIter last1,
    typename iterator_traits<BidirIter>::difference_type d1,
    BidirIter first2,
    BidirIter last2,
    typename iterator_traits<BidirIter>::difference_type d2,
    BidirIter first3,
    BidirIter last3,
    typename iterator_traits<BidirIter>::difference_type d3,
    Function &f)
{
    using F = call_combine_discontinuous<Function&, BidirIter>;
    F fbc(first2, last2, d2, first3, last3, d3, f);  // BC
    return combine_discontinuous3_(first1, last1, d1, first2, last2, d2, first3, last3, d3, fbc);
}

/**
 *  \brief See permute.
 */
template <typename BidirIter, typename Function>
bool permute_(BidirIter first1,
    BidirIter last1,
    typename iterator_traits<BidirIter>::difference_type d1,
    Function &f)
{
    switch (d1) {
        case 0:
        case 1:
            return f();
        case 2:
            if (f()) {
                return true;
            }
            swap(*first1, *next(first1));
            return f();
        case 3: {
            if (f()) {
                return true;
            }
            BidirIter f2 = next(first1);
            BidirIter f3 = next(f2);
            swap(*f2, *f3);
            if (f()) {
                return true;
            }
            swap(*first1, *f3);
            swap(*f2, *f3);
            if (f()) {
                return true;
            }
            swap(*f2, *f3);
            if (f()) {
                return true;
            }
            swap(*first1, *f2);
            swap(*f2, *f3);
            if (f()) {
                return true;
            }
            swap(*f2, *f3);
            return f();
        }
    }

    BidirIter fp1 = next(first1);
    for (BidirIter p = fp1; p != last1; ++p)
    {
        if (permute_(fp1, last1, d1-1, f)) {
            return true;
        }
        reverse(fp1, last1);
        swap(*first1, *p);
    }

    return permute_(fp1, last1, d1-1, f);
}


/**
 *  Calls f() for each permutation of [first1, last1)
 * Divided into permute and permute_ in a (perhaps futile) attempt to
 * squeeze a little more performance out of it.
 */
template <typename BidirIter, typename Function>
bool permute(BidirIter first1,
    BidirIter last1,
    typename iterator_traits<BidirIter>::difference_type d1,
    Function &f)
{
    switch (d1) {
        case 0:
        case 1:
            return f();
        case 2:
            {
            if (f()) {
                return true;
            }
            BidirIter i = next(first1);
            swap(*first1, *i);
            if (f()) {
                return true;
            }
            swap(*first1, *i);
            }
            break;
        case 3:
            {
            if (f()) {
                return true;
            }
            BidirIter f2 = next(first1);
            BidirIter f3 = next(f2);
            swap(*f2, *f3);
            if (f()) {
                return true;
            }
            swap(*first1, *f3);
            swap(*f2, *f3);
            if (f()) {
                return true;
            }
            swap(*f2, *f3);
            if (f()) {
                return true;
            }
            swap(*first1, *f2);
            swap(*f2, *f3);
            if (f()) {
                return true;
            }
            swap(*f2, *f3);
            if (f()) {
                return true;
            }
            swap(*first1, *f3);
            }
            break;
        default:
            BidirIter fp1 = next(first1);
            for (BidirIter p = fp1; p != last1; ++p)
            {
                if (permute_(fp1, last1, d1-1, f)) {
                    return true;
                }
                reverse(fp1, last1);
                swap(*first1, *p);
            }
            if (permute_(fp1, last1, d1-1, f)) {
                return true;
            }
            reverse(first1, last1);
            break;
    }
    return false;
}


/**
 *  \brief Creates a functor with no arguments which calls f_(first_, last_).
 *   Also has a variant that takes two It and ignores them.
 */
template <typename Function, typename It>
class bound_range
{
protected:
    Function f_;
    It first_;
    It last_;

public:
    bound_range(Function f,
            It first,
            It last):
        f_(f),
        first_(first),
        last_(last)
    {}

    bool operator()()
    {
        return f_(first_, last_);
    }

    bool operator()(It, It)
    {
        return f_(first_, last_);
    }
};


/**
 *  \brief A binder for binding arguments to call permute.
 */
template <typename Function, typename It>
class call_permute
{
protected:
    using D = typename iterator_traits<It>::difference_type;
    Function f_;
    It first_;
    It last_;
    D d_;

public:
    call_permute(Function f,
            It first,
            It last,
            D d):
        f_(f),
        first_(first),
        last_(last),
        d_(d)
    {}

    bool operator()()
    {
        return permute(first_, last_, d_, f_);
    }
};


template <typename BidirIter, typename Function>
Function for_each_combination(BidirIter first,
    BidirIter mid,
    BidirIter last,
    Function f)
{
    bound_range<Function&, BidirIter> wfunc(f, first, mid);
    combine_discontinuous(first, mid, distance(first, mid), mid, last, distance(mid, last), wfunc);
    return move(f);
}


template <typename UInt>
UInt gcd(UInt x, UInt y)
{
    while (y != 0)
    {
        UInt t = x % y;
        x = y;
        y = t;
    }
    return x;
}


template <typename UInt>
UInt count_each_combination(UInt d1, UInt d2)
{
    if (d2 < d1) {
        swap(d1, d2);
    }
    if (d1 == 0) {
        return 1;
    }
    if (d1 > numeric_limits<UInt>::max() - d2) {
        throw overflow_error("overflow in count_each_combination");
    }
    UInt n = d1 + d2;
    UInt r = n;
    --n;
    for (UInt k = 2; k <= d1; ++k, --n) {
        // r = r * n / k, known to not not have truncation error
        UInt g = gcd(r, k);
        r /= g;
        UInt t = n / (k / g);
        if (r > numeric_limits<UInt>::max() / t) {
            throw overflow_error("overflow in count_each_combination");
        }
        r *= t;
    }
    return r;
}


/**
 *  \brief Sample all permutations.
 *
 *  For each of the permutation algorithms, use for_each_combination (or
 *  combine_discontinuous) to handle the "r out of N" part of the algorithm.
 *  Thus each permutation algorithm has to deal only with an "N out of N"
 *  problem.  I.e. For each combination of r out of N items, permute it
 *  thusly.
 */
template <typename BidirIter, typename Function>
Function for_each_permutation(BidirIter first,
    BidirIter mid,
    BidirIter last,
    Function f)
{
    using D = typename iterator_traits<BidirIter>::difference_type;
    using Wf = bound_range<Function&, BidirIter>;
    using PF = call_permute<Wf, BidirIter>;

    Wf wfunc(f, first, mid);
    D d1 = distance(first, mid);
    PF pf(wfunc, first, mid, d1);
    combine_discontinuous(first, mid, d1, mid, last, distance(mid, last), pf);
    return move(f);
}


template <typename UInt>
UInt count_each_permutation(UInt d1, UInt d2)
{
    // return (d1+d2)!/d2!
    if (d1 > numeric_limits<UInt>::max() - d2) {
        throw overflow_error("overflow in count_each_permutation");
    }
    UInt n = d1 + d2;
    UInt r = 1;
    for (; n > d2; --n) {
        if (r > numeric_limits<UInt>::max() / n) {
            throw overflow_error("overflow in count_each_permutation");
        }
        r *= n;
    }
    return r;
}


template <typename BidirIter>
uintmax_t count_each_permutation(BidirIter first,
    BidirIter mid,
    BidirIter last)
{
    return count_each_permutation<uintmax_t>(distance(first, mid), distance(mid, last));
}


/**
 *  \brief Adapt functor to permute over [first+1, last)
 *  A circular permutation of N items is done by holding the first item and
 *  permuting [first+1, last).
 */
template <typename Function, typename BidirIter>
class circular_permutation
{
protected:
    using D = typename iterator_traits<BidirIter>::difference_type;

    Function f_;
    D s_;

public:
    explicit circular_permutation(Function f, D s) : f_(f), s_(s) {}

    bool operator()(BidirIter first,
        BidirIter last)
    {
        if (s_ <= 1) {
            return f_(first, last);
        }
        bound_range<Function, BidirIter> f(f_, first, last);
        return permute(next(first), last, s_ - 1, f);
    }
};


template <typename BidirIter, typename Function>
Function for_each_circular_permutation(BidirIter first,
    BidirIter mid,
    BidirIter last,
    Function f)
{
    for_each_combination(first, mid, last, circular_permutation<Function&, BidirIter>(f, distance(first, mid)));
    return move(f);
}


template <typename UInt>
UInt count_each_circular_permutation(UInt d1, UInt d2)
{
    // return d1 > 0 ? (d1+d2)!/(d1*d2!) : 1
    if (d1 == 0) {
        return 1;
    }

    UInt r;
    if (d1 <= d2) {
        try {
            r = count_each_combination(d1, d2);
        } catch (const overflow_error&) {
            throw overflow_error("overflow in count_each_circular_permutation");
        }
        for (--d1; d1 > 1; --d1) {
            if (r > numeric_limits<UInt>::max()/d1)
                throw overflow_error("overflow in count_each_circular_permutation");
            r *= d1;
        }
    } else {   // functionally equivalent but faster algorithm
        if (d1 > numeric_limits<UInt>::max() - d2) {
            throw overflow_error("overflow in count_each_circular_permutation");
        }
        UInt n = d1 + d2;
        r = 1;
        for (; n > d1; --n) {
            if (r > numeric_limits<UInt>::max()/n) {
                throw overflow_error("overflow in count_each_circular_permutation");
            }
            r *= n;
        }

        for (--n; n > d2; --n) {
            if (r > numeric_limits<UInt>::max()/n) {
                throw overflow_error("overflow in count_each_circular_permutation");
            }
            r *= n;
        }
    }
    return r;
}


template <typename BidirIter>
uintmax_t count_each_circular_permutation(BidirIter first,
    BidirIter mid,
    BidirIter last)
{
    return count_each_circular_permutation<uintmax_t>(distance(first, mid), distance(mid, last));
}


/**
 *  \warning Difficult!!!  See notes for operator().
 */
template <typename Function, typename Size>
class reversible_permutation
{
protected:
    Function f_;
    Size s_;

public:
    reversible_permutation(Function f, Size s):
        f_(f),
        s_(s)
    {}

    template <typename BidirIter>
    bool operator()(BidirIter first, BidirIter last);
};


/**
 *  rev1 looks like call_permute
 */
template <typename Function, typename BidirIter>
class rev1
{
protected:
    using D = typename iterator_traits<BidirIter>::difference_type;

    Function f_;
    BidirIter first1_;
    BidirIter last1_;
    D d1_;

public:
    rev1(Function f, BidirIter first, BidirIter last, D d):
        f_(f),
        first1_(first),
        last1_(last),
        d1_(d)
    {}

    bool operator()()
    {
        return permute(first1_, last1_, d1_, f_);
    }
};


/**
 *  For each permutation in [first1, last1),
 *  call f() for each permutation of [first2, last2).
 */
template <typename Function, typename BidirIter>
class rev2
{
protected:
    using D = typename iterator_traits<BidirIter>::difference_type;

    Function f_;
    BidirIter first1_;
    BidirIter last1_;
    D d1_;
    BidirIter first2_;
    BidirIter last2_;
    D d2_;

public:
    rev2(Function f, BidirIter first1, BidirIter last1,
         D d1, BidirIter first2, BidirIter last2, D d2):
        f_(f),
        first1_(first1),
        last1_(last1),
        d1_(d1),
        first2_(first2),
        last2_(last2),
        d2_(d2)
    {}

    bool operator()()
    {
        call_permute<Function, BidirIter> f(f_, first2_, last2_, d2_);
        return permute(first1_, last1_, d1_, f);
    }
};


/**
 *  For each permutation in [first1, last1),
 *  and for each permutation of [first2, last2)
 *  call f() for each permutation of [first3, last3).
 */
template <typename Function, typename BidirIter>
class rev3
{
protected:
    using D = typename iterator_traits<BidirIter>::difference_type;

    Function f_;
    BidirIter first1_;
    BidirIter last1_;
    D d1_;
    BidirIter first2_;
    BidirIter last2_;
    D d2_;
    BidirIter first3_;
    BidirIter last3_;
    D d3_;

public:
    rev3(Function f, BidirIter first1, BidirIter last1,
         D d1, BidirIter first2, BidirIter last2,
         D d2, BidirIter first3, BidirIter last3, D d3):
        f_(f),
        first1_(first1),
        last1_(last1),
        d1_(d1),
        first2_(first2),
        last2_(last2), d2_(d2),
        first3_(first3),
        last3_(last3), d3_(d3)
    {}

    bool operator()()
    {
        rev2<Function, BidirIter> f(f_, first2_, last2_, d2_, first3_, last3_, d3_);
        return permute(first1_, last1_, d1_, f);
    }
};


/**
 *  There are simpler implementations.  I believe the simpler ones are
 *  far more expensive.
 */
template <typename Function, typename Size>
template <typename BidirIter>
bool reversible_permutation<Function, Size>::operator()(BidirIter first,
    BidirIter last)
{
    using difference_type = typename iterator_traits<BidirIter>::difference_type;
    using F2 = rev2<bound_range<Function&, BidirIter>, BidirIter>;
    using F3 = rev3<bound_range<Function&, BidirIter>, BidirIter>;

    // When the range is 0 - 2, then this is just a combination of N out of N
    //   elements.
    if (s_ < 3) {
        return f_(first, last);
    }

    // Hold the first element steady and call f_(first, last) for each
    //    permutation in [first+1, last).
    BidirIter a = next(first);
    bound_range<Function&, BidirIter> f(f_, first, last);
    if (permute(a, last, s_-1, f)) {
        return true;
    }

    // Beginning with the first element, swap the previous element with the
    //    next element.  For each swap, call f_(first, last) for each
    //    permutation of the discontinuous range:
    //    [prior to the orignal element] + [after the original element].
    Size s2 = s_ / 2;
    BidirIter am1 = first;
    BidirIter ap1 = next(a);
    for (Size i = 1; i < s2; ++i, ++am1, ++a, ++ap1) {
        swap(*am1, *a);
        F2 f2(f, first, a, i, ap1, last, s_ - i - 1);
        if (combine_discontinuous(first, a, i, ap1, last, s_ - i - 1, f2)) {
            return true;
        }
    }

    // If [first, last) has an even number of elements, then fix it up to the
    //     original permutation.
    if (2 * s2 == s_) {
        rotate(first, am1, a);
    } else if (s_ == 3) {
        // else if the range has length 3, we need one more call and the fix is easy.
        swap(*am1, *a);
        if (f_(first, last)) {
            return true;
        }
        swap(*am1, *a);
    } else {
        // else the range is an odd number greater than 3.  We need to permute
        //     through exactly half of the permuations with the original element in
        //     the middle.

        // swap the original first element into the middle, and hold the current
        //   first element steady.  This creates a discontinuous range:
        //     [first+1, middle) + [middle+1, last).  Run through all permutations
        //     of that discontinuous range.
        swap(*am1, *a);
        BidirIter b = first;
        BidirIter bp1 = next(b);
        F2 f2(f, bp1, a, s2-1, ap1, last, s_ - s2 - 1);
        if (combine_discontinuous(bp1, a, s2-1, ap1, last, s_ - s2 - 1, f2)) {
            return true;
        }
        // Swap the current first element into every place from first+1 to middle-1.
        //   For each location, hold it steady to create the following discontinuous
        //   range (made of 3 ranges): [first, b-1) + [b+1, middle) + [middle+1, last).
        //   For each b in [first+1, middle-1), run through all permutations of
        //      the discontinuous ranges.
        b = bp1;
        ++bp1;
        BidirIter bm1 = first;
        for (Size i = 1; i < s2-1; ++i, ++bm1, ++b, ++bp1)
        {
            swap(*bm1, *b);
            F3 f3(f, first, b, i, bp1, a, s2-i-1, ap1, last, s_ - s2 - 1);
            if (combine_discontinuous3(first, b, i, bp1, a, s2-i-1, ap1, last, s_-s2-1, f3)) {
                return true;
            }
        }
        // swap b into into middle-1, creates a discontinuous range:
        //     [first, middle-1) + [middle+1, last).  Run through all permutations
        //     of that discontinuous range.
        swap(*bm1, *b);
        F2 f21(f, first, b, s2-1, ap1, last, s_ - s2 - 1);
        if (combine_discontinuous(first, b, s2-1, ap1, last, s_ - s2 - 1, f21)) {
            return true;
        }
        // Revert [first, last) to original order
        reverse(first, b);
        reverse(first, ap1);
    }
    return false;
}


template <typename BidirIter, typename Function>
Function for_each_reversible_permutation(BidirIter first,
    BidirIter mid,
    BidirIter last,
    Function f)
{
    using D = typename iterator_traits<BidirIter>::difference_type;

    for_each_combination(first, mid, last, reversible_permutation<Function&, D>(f, distance(first, mid)));
    return move(f);
}


template <typename UInt>
UInt count_each_reversible_permutation(UInt d1, UInt d2)
{
    // return d1 > 1 ? (d1+d2)!/(2*d2!) : (d1+d2)!/d2!
    if (d1 > numeric_limits<UInt>::max() - d2) {
        throw overflow_error("overflow in count_each_reversible_permutation");
    }
    UInt n = d1 + d2;
    UInt r = 1;
    if (d1 > 1) {
        r = n;
        if ((n & 1) == 0) {
            r /= 2;
        }
        --n;
        UInt t = n;
        if ((t & 1) == 0) {
            t /= 2;
        }
        if (r > numeric_limits<UInt>::max() / t) {
            throw overflow_error("overflow in count_each_reversible_permutation");
        }
        r *= t;
        --n;
    }
    for (; n > d2; --n) {
        if (r > numeric_limits<UInt>::max() / n) {
            throw overflow_error("overflow in count_each_reversible_permutation");
        }
        r *= n;
    }
    return r;
}


template <typename BidirIter>
uintmax_t count_each_reversible_permutation(BidirIter first, BidirIter mid, BidirIter last)
{
    return count_each_reversible_permutation<uintmax_t>(distance(first, mid), distance(mid, last));
}


/**
 *  \brief Adapt functor to permute over [first+1, last)
 *
 *   A reversible circular permutation of N items is done by holding the first
 *   item and reverse-permuting [first+1, last).
 */
template <typename Function, typename BidirIter>
class reverse_circular_permutation
{
    using D = typename iterator_traits<BidirIter>::difference_type;

    Function f_;
    D s_;

public:
    explicit reverse_circular_permutation(Function f,
            D s):
        f_(f),
        s_(s)
    {}

    bool
    operator()(BidirIter first,
        BidirIter last)
    {
        if (s_ == 1) {
            return f_(first, last);
        }
        using D = typename iterator_traits<BidirIter>::difference_type;
        using BoundFunc = bound_range<Function, BidirIter>;
        BoundFunc f(f_, first, last);
        BidirIter n = next(first);
        return reversible_permutation<BoundFunc, D>(f, distance(n, last))(n, last);
    }
};


template <typename BidirIter, typename Function>
Function for_each_reversible_circular_permutation(BidirIter first,
    BidirIter mid,
    BidirIter last,
    Function f)
{
    for_each_combination(first, mid, last, reverse_circular_permutation<Function&, BidirIter>(f, distance(first, mid)));
    return move(f);
}

template <typename UInt>
UInt count_each_reversible_circular_permutation(UInt d1, UInt d2)
{
    // return d1 == 0 ? 1 : d1 <= 2 ? (d1+d2)!/(d1*d2!) : (d1+d2)!/(2*d1*d2!)
    UInt r;
    try {
        r = count_each_combination(d1, d2);
    } catch (const overflow_error&) {
        throw overflow_error("overflow in count_each_reversible_circular_permutation");
    }
    if (d1 > 3) {
        for (--d1; d1 > 2; --d1) {
            if (r > numeric_limits<UInt>::max()/d1) {
                throw overflow_error("overflow in count_each_reversible_circular_permutation");
            }
            r *= d1;
        }
    }
    return r;
}


template <typename BidirIter>
uintmax_t count_each_reversible_circular_permutation(BidirIter first,
    BidirIter mid,
    BidirIter last)
{
    return count_each_reversible_circular_permutation<uintmax_t>(distance(first, mid), distance(mid, last));
}

PYCPP_END_NAMESPACE
