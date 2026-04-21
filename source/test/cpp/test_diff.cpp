#include "ccore/c_allocator.h"

#include "cgx2/c_diff.h"

#include "cunittest/cunittest.h"

using namespace ncore;

UNITTEST_SUITE_BEGIN(gx2)
{
    UNITTEST_FIXTURE(diff)
    {
        UNITTEST_FIXTURE_SETUP() {}
        UNITTEST_FIXTURE_TEARDOWN() {}

        UNITTEST_ALLOCATOR;

        UNITTEST_TEST(create)
        {
        }

    }
}
UNITTEST_SUITE_END
