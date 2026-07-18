#include "ccore/c_allocator.h"

#include "cgx2/c_draw.h"

#include "cunittest/cunittest.h"

using namespace ncore;

UNITTEST_SUITE_BEGIN(gx2)
{
    UNITTEST_FIXTURE(draw)
    {
        UNITTEST_FIXTURE_SETUP() {}
        UNITTEST_FIXTURE_TEARDOWN() {}

        UNITTEST_TEST(pixel)
        {
        }

        UNITTEST_TEST(clipped_line)
        {
            ngx2::color_t       pixels[8 * 8] = {};
            ngx2::framebuffer_t fb             = {ngx2::init_image_descr(8, 8, ngx2::IMAGE_FORMAT_RGB565), pixels};
            ngx2::rect_t        scissor        = {2, 2, 4, 4};

            ngx2::draw_line(fb, scissor, -3, 3, 9, 3, 0x1234);
            for (i32 x = 0; x < 8; ++x)
                CHECK(pixels[x + 3 * 8] == (x >= 2 && x <= 5 ? 0x1234 : 0));

            ngx2::draw_line(fb, scissor, 0, 0, 7, 0, 0x5678);
            for (i32 x = 0; x < 8; ++x)
                CHECK(pixels[x] == 0);

            ngx2::draw_line(fb, scissor, 7, 7, 0, 0, 0x9abc);
            for (i32 p = 2; p <= 5; ++p)
                CHECK(pixels[p + p * 8] == 0x9abc);
        }

        UNITTEST_TEST(text_metrics_and_coverage)
        {
            const u8  coverage[] = {0, 128, 255, 255};
            const u32 offsets[]  = {0, 3};
            ngx2::glyph_t glyphs[] = {
                {4, 1, -1, 3, 1},
                {1, 0, 0, 1, 1},
            };
            ngx2::font_t font = {};
            font.coverage     = coverage;
            font.glyphs       = glyphs;
            font.offsets      = offsets;
            font.bpp          = 8;
            for (i32 i = 0; i < 256; ++i)
                font.map[i] = 0xff;
            font.map['A'] = 0;
            font.map['B'] = 1;

            ngx2::color_t       pixels[8 * 5] = {};
            ngx2::framebuffer_t fb             = {ngx2::init_image_descr(8, 5, ngx2::IMAGE_FORMAT_RGB565), pixels};
            ngx2::rect_t        scissor        = {0, 0, 8, 5};

            ngx2::draw_text(fb, scissor, &font, 1, 3, "AB", 0xffff);

            CHECK(pixels[2 + 2 * 8] == 0x0000);
            CHECK(pixels[3 + 2 * 8] == 0x8410);
            CHECK(pixels[4 + 2 * 8] == 0xffff);
            CHECK(pixels[5 + 3 * 8] == 0xffff);
        }

        UNITTEST_TEST(text_clipping)
        {
            const u8  coverage[] = {0x07, 0x07, 0x07};
            const u32 offsets[]  = {0};
            ngx2::glyph_t glyphs[] = {
                {3, 0, 0, 3, 3},
            };
            ngx2::font_t font = {};
            font.coverage     = coverage;
            font.glyphs       = glyphs;
            font.offsets      = offsets;
            font.bpp          = 1;
            for (i32 i = 0; i < 256; ++i)
                font.map[i] = 0xff;
            font.map['A'] = 0;

            ngx2::color_t       pixels[4 * 4] = {};
            ngx2::framebuffer_t fb             = {ngx2::init_image_descr(4, 4, ngx2::IMAGE_FORMAT_RGB565), pixels};
            ngx2::rect_t        scissor        = {1, 1, 1, 1};

            ngx2::draw_text(fb, scissor, &font, 0, 0, "A", 0x1234);

            for (i32 y = 0; y < 4; ++y)
            {
                for (i32 x = 0; x < 4; ++x)
                    CHECK(pixels[x + y * 4] == (x == 1 && y == 1 ? 0x1234 : 0));
            }

            scissor = {-2, -2, 6, 6};
            ngx2::draw_text(fb, scissor, &font, -2, -2, "A", 0x5678);
            CHECK(pixels[0] == 0x5678);
        }

        UNITTEST_TEST(text_mapping_and_advance)
        {
            const u8  coverage[] = {0x03};
            const u32 offsets[]  = {0, 1};
            ngx2::glyph_t glyphs[] = {
                {1, 0, 0, 1, 1},
                {2, 0, 0, 0, 0},
            };
            ngx2::font_t font = {};
            font.coverage     = coverage;
            font.glyphs       = glyphs;
            font.offsets      = offsets;
            font.bpp          = 2;
            for (i32 i = 0; i < 256; ++i)
                font.map[i] = 0xff;
            font.map['A'] = 0;
            font.map['B'] = 1;

            ngx2::color_t       pixels[6] = {};
            ngx2::framebuffer_t fb         = {ngx2::init_image_descr(6, 1, ngx2::IMAGE_FORMAT_RGB565), pixels};
            ngx2::rect_t        scissor    = {0, 0, 6, 1};
            const char          text[]     = {(char)0x80, 'B', 'A', 0};

            ngx2::draw_text(fb, scissor, &font, 1, 0, text, 0xabcd);

            for (i32 x = 0; x < 6; ++x)
                CHECK(pixels[x] == (x == 3 ? 0xabcd : 0));
        }

        UNITTEST_TEST(text_packed_coverage)
        {
            const u8  coverage[] = {0x1b, 0x0f, 0xf1};
            const u32 offsets[]  = {0};
            ngx2::glyph_t glyphs[] = {
                {4, 0, 0, 4, 1},
            };
            ngx2::font_t font = {};
            font.coverage     = coverage;
            font.glyphs       = glyphs;
            font.offsets      = offsets;
            for (i32 i = 0; i < 256; ++i)
                font.map[i] = 0xff;
            font.map['A'] = 0;

            ngx2::color_t       pixels[4] = {};
            ngx2::framebuffer_t fb         = {ngx2::init_image_descr(4, 1, ngx2::IMAGE_FORMAT_RGB565), pixels};
            ngx2::rect_t        scissor    = {0, 0, 4, 1};

            font.bpp = 2;
            ngx2::draw_text(fb, scissor, &font, 0, 0, "A", 0xffff);
            CHECK(pixels[0] == 0xffff);
            CHECK(pixels[1] == 0x7bef);
            CHECK(pixels[2] == 0x39e7);
            CHECK(pixels[3] == 0x0000);

            for (i32 i = 0; i < 4; ++i)
                pixels[i] = 0;
            font.coverage = coverage + 1;
            font.bpp = 4;
            ngx2::draw_text(fb, scissor, &font, 0, 0, "A", 0xffff);
            CHECK(pixels[0] == 0xffff);
            CHECK(pixels[1] == 0x0000);
            CHECK(pixels[2] == 0x0861);
            CHECK(pixels[3] == 0xffff);
        }

        UNITTEST_TEST(text_packed_clipping_offsets)
        {
            const u32 offsets[] = {0};
            ngx2::glyph_t glyphs[] = {
                {10, 0, 0, 10, 1},
            };
            ngx2::font_t font = {};
            font.glyphs       = glyphs;
            font.offsets      = offsets;
            for (i32 i = 0; i < 256; ++i)
                font.map[i] = 0xff;
            font.map['A'] = 0;

            ngx2::color_t       pixels[6] = {};
            ngx2::framebuffer_t fb         = {ngx2::init_image_descr(6, 1, ngx2::IMAGE_FORMAT_RGB565), pixels};
            ngx2::rect_t        scissor    = {0, 0, 6, 1};

            const u8 coverage_1[] = {0x08, 0x01};
            font.coverage           = coverage_1;
            font.bpp                = 1;
            ngx2::draw_text(fb, scissor, &font, -3, 0, "A", 0xffff);
            for (i32 i = 0; i < 6; ++i)
                CHECK(pixels[i] == (i == 0 || i == 5 ? 0xffff : 0));

            for (i32 i = 0; i < 6; ++i)
                pixels[i] = 0;
            const u8 coverage_2[] = {0xc0, 0x0c, 0x00};
            font.coverage           = coverage_2;
            font.bpp                = 2;
            ngx2::draw_text(fb, scissor, &font, -3, 0, "A", 0xffff);
            CHECK(pixels[0] == 0xffff);
            CHECK(pixels[1] == 0x0000);
            CHECK(pixels[2] == 0xffff);

            for (i32 i = 0; i < 6; ++i)
                pixels[i] = 0;
            const u8 coverage_4[] = {0xf0, 0xf0, 0x00, 0x00, 0x00};
            font.coverage           = coverage_4;
            font.bpp                = 4;
            ngx2::draw_text(fb, scissor, &font, -1, 0, "A", 0xffff);
            CHECK(pixels[0] == 0xffff);
            CHECK(pixels[1] == 0x0000);
            CHECK(pixels[2] == 0xffff);

            for (i32 i = 0; i < 6; ++i)
                pixels[i] = 0;
            const u8 coverage_8[] = {0, 255, 0, 255, 0, 0, 0, 0, 0, 0};
            font.coverage           = coverage_8;
            font.bpp                = 8;
            ngx2::draw_text(fb, scissor, &font, -1, 0, "A", 0xffff);
            CHECK(pixels[0] == 0xffff);
            CHECK(pixels[1] == 0x0000);
            CHECK(pixels[2] == 0xffff);
        }

        UNITTEST_TEST(text_invalid_inputs)
        {
            ngx2::color_t       pixels[2] = {0x1111, 0x2222};
            ngx2::framebuffer_t fb         = {ngx2::init_image_descr(2, 1, ngx2::IMAGE_FORMAT_RGB565), pixels};
            ngx2::rect_t        scissor    = {0, 0, 2, 1};
            ngx2::font_t        font       = {};

            ngx2::draw_text(fb, scissor, nullptr, 0, 0, "A", 0xffff);
            ngx2::draw_text(fb, scissor, &font, 0, 0, nullptr, 0xffff);
            ngx2::draw_text(fb, scissor, &font, 0, 0, "", 0xffff);

            CHECK(pixels[0] == 0x1111);
            CHECK(pixels[1] == 0x2222);
        }
    }
}
UNITTEST_SUITE_END
