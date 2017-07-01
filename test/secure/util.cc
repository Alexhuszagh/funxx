//  :copyright: (c) 2017 Alex Huszagh.
//  :license: MIT, see licenses/mit.md for more details.

#include <secure/util.h>
#include <gtest/gtest.h>


// TESTS
// -----


TEST(secure_util, secure_malloc)
{
    auto* ptr = secure_malloc(50);
    secure_free(ptr);
}


TEST(secure_util, secure_calloc)
{
    auto* ptr = secure_calloc(50, 1);
    secure_free(ptr);
}
