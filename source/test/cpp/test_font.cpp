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
            const u8 font_size = 32u;
            const u8 ascent    = 20u;

            CHECK_EQUAL(80u, ngx2::get_text_y_start(font_size, ascent, 100u, 32));
            CHECK_EQUAL(90u, ngx2::get_text_y_start(font_size, ascent, 100u, 16));
        }

        UNITTEST_TEST(y_end)
        {
            const u8 font_size = 32u;
            const u8 descent   = 4u;

            CHECK_EQUAL(104u, ngx2::get_text_y_end(font_size, descent, 100u, 32));
            CHECK_EQUAL(106u, ngx2::get_text_y_end(font_size, descent, 100u, 48));
        }
    }
}
UNITTEST_SUITE_END
