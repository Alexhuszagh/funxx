//  :copyright: (c) 2017 Alex Huszagh.
//  :license: MIT, see LICENSE.md for more details.
/*
 *  \addtogroup Tests
 *  \brief Processor detection unittests.
 */

#include <pycpp/preprocessor/processor.h>
#include <gtest/gtest.h>

// TESTS
// -----


TEST(processor, system_architecture)
{
#ifndef PROCESSOR_DETECTED
#   error "Processor not detected."
#endif
}
