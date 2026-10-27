#include "ccore/c_allocator.h"

#include "cgx2/c_font.h"

#include "cunittest/cunittest.h"

using namespace ncore;

UNITTEST_SUITE_BEGIN(font)
{
    UNITTEST_FIXTURE(main)
    {
        UNITTEST_FIXTURE_SETUP() {}
        UNITTEST_FIXTURE_TEARDOWN() {}

        UNITTEST_TEST(y_start)
        {
            const u16 ascent      = 20u << 8;

            CHECK_EQUAL(80u << 8, ngx2::get_text_y_start(ascent, 100u << 8, 32));
            CHECK_EQUAL(90u << 8, ngx2::get_text_y_start(ascent, 100u << 8, 16));
        }

        UNITTEST_TEST(y_end)
        {
            const u16 descent     = 4u << 8;

            CHECK_EQUAL(104u << 8, ngx2::get_text_y_end(descent, 100u << 8, 32));
            CHECK_EQUAL(106u << 8, ngx2::get_text_y_end(descent, 100u << 8, 48));
        }
    }
}
UNITTEST_SUITE_END
