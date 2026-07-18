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

        UNITTEST_TEST(text_metrics_and_sdf)
        {
            const u8  sdf[]     = {0x18, 0xf0, 0xf0};
            const u32 offsets[] = {0, 2};
            ngx2::glyph_t glyphs[] = {
                {4, 1, 1, 3, 1},
                {1, 0, 1, 1, 1},
            };
            ngx2::font_t font = {};
            font.m_sdf        = sdf;
            font.m_glyphs     = glyphs;
            font.m_offsets    = offsets;
            for (i32 i = 0; i < 128; ++i)
                font.m_map[i] = 0xff;
            font.m_map['A'] = 0;
            font.m_map['B'] = 1;

            ngx2::color_t       pixels[8 * 3] = {};
            ngx2::framebuffer_t fb             = {ngx2::init_image_descr(8, 3, ngx2::IMAGE_FORMAT_RGB565), pixels};

            ngx2::draw_text(fb, &font, 1, 2, "AB", 0xffff, 1.0f);

            CHECK(pixels[2 + 1 * 8] == 0x0000);
            CHECK(pixels[3 + 1 * 8] == 0x6b6d);
            CHECK(pixels[4 + 1 * 8] == 0xdefb);
            CHECK(pixels[5 + 1 * 8] == 0xdefb);
        }

        UNITTEST_TEST(text_mapping_and_advance)
        {
            const u8  sdf[]     = {0xf0};
            const u32 offsets[] = {0, 1};
            ngx2::glyph_t glyphs[] = {
                {1, 0, 0, 1, 1},
                {2, 0, 0, 0, 0},
            };
            ngx2::font_t font = {};
            font.m_sdf        = sdf;
            font.m_glyphs     = glyphs;
            font.m_offsets    = offsets;
            for (i32 i = 0; i < 128; ++i)
                font.m_map[i] = 0xff;
            font.m_map['A'] = 0;
            font.m_map['B'] = 1;

            ngx2::color_t       pixels[6] = {};
            ngx2::framebuffer_t fb         = {ngx2::init_image_descr(6, 1, ngx2::IMAGE_FORMAT_RGB565), pixels};
            const char          text[]     = {(char)0x80, 'B', 'A', 0};

            ngx2::draw_text(fb, &font, 1, 0, text, 0xffff, 1.0f);

            for (i32 x = 0; x < 6; ++x)
                CHECK(pixels[x] == (x == 3 ? 0xdefb : 0));
        }

        UNITTEST_TEST(text_packed_sdf)
        {
            const u8  sdf[]     = {0x29, 0xaf};
            const u32 offsets[] = {0};
            ngx2::glyph_t glyphs[] = {
                {4, 0, 0, 4, 1},
            };
            ngx2::font_t font = {};
            font.m_sdf        = sdf;
            font.m_glyphs     = glyphs;
            font.m_offsets    = offsets;
            for (i32 i = 0; i < 128; ++i)
                font.m_map[i] = 0xff;
            font.m_map['A'] = 0;

            ngx2::color_t       pixels[4] = {};
            ngx2::framebuffer_t fb         = {ngx2::init_image_descr(4, 1, ngx2::IMAGE_FORMAT_RGB565), pixels};

            ngx2::draw_text(fb, &font, 0, 0, "A", 0xffff, 1.0f);

            CHECK(pixels[0] == 0x0861);
            CHECK(pixels[1] == 0x7bef);
            CHECK(pixels[2] == 0x8c71);
            CHECK(pixels[3] == 0xdefb);
        }

        UNITTEST_TEST(text_sdf_offsets)
        {
            const u8  sdf[]     = {0xf0, 0xf0, 0x2f};
            const u32 offsets[] = {0, 2};
            ngx2::glyph_t glyphs[] = {
                {4, 0, 0, 3, 1},
                {2, 0, 0, 2, 1},
            };
            ngx2::font_t font = {};
            font.m_sdf        = sdf;
            font.m_glyphs     = glyphs;
            font.m_offsets    = offsets;
            for (i32 i = 0; i < 128; ++i)
                font.m_map[i] = 0xff;
            font.m_map['A'] = 0;
            font.m_map['B'] = 1;

            ngx2::color_t       pixels[6] = {};
            ngx2::framebuffer_t fb         = {ngx2::init_image_descr(6, 1, ngx2::IMAGE_FORMAT_RGB565), pixels};

            ngx2::draw_text(fb, &font, 0, 0, "AB", 0xffff, 1.0f);

            CHECK(pixels[0] == 0xdefb);
            CHECK(pixels[1] == 0x0000);
            CHECK(pixels[2] == 0xdefb);
            CHECK(pixels[3] == 0x0000);
            CHECK(pixels[4] == 0x0861);
            CHECK(pixels[5] == 0xdefb);
        }

        UNITTEST_TEST(text_scale)
        {
            const u8  sdf[]     = {0x2f};
            const u32 offsets[] = {0};
            ngx2::glyph_t glyphs[] = {
                {2, 0, 0, 2, 1},
            };
            ngx2::font_t font = {};
            font.m_sdf        = sdf;
            font.m_glyphs     = glyphs;
            font.m_offsets    = offsets;
            for (i32 i = 0; i < 128; ++i)
                font.m_map[i] = 0xff;
            font.m_map['A'] = 0;

            ngx2::color_t       pixels[8 * 2] = {};
            ngx2::framebuffer_t fb             = {ngx2::init_image_descr(8, 2, ngx2::IMAGE_FORMAT_RGB565), pixels};

            ngx2::draw_text(fb, &font, 0, 0, "AA", 0xffff, 2.0f);

            const ngx2::color_t expected[] = {0x0861, 0x0861, 0xdefb, 0xdefb, 0x0861, 0x0861, 0xdefb, 0xdefb};
            for (i32 y = 0; y < 2; ++y)
            {
                for (i32 x = 0; x < 8; ++x)
                    CHECK(pixels[x + y * 8] == expected[x]);
            }
        }

        UNITTEST_TEST(text_newline)
        {
            const u8  sdf[]     = {0xf0};
            const u32 offsets[] = {0};
            ngx2::glyph_t glyphs[] = {
                {2, 0, 0, 1, 1},
            };
            ngx2::font_t font = {};
            font.m_sdf        = sdf;
            font.m_glyphs     = glyphs;
            font.m_offsets    = offsets;
            font.m_ascent     = 2;
            font.m_descent    = -1;
            font.m_line_gap   = 1;
            for (i32 i = 0; i < 128; ++i)
                font.m_map[i] = 0xff;
            font.m_map['A'] = 0;

            ngx2::color_t       pixels[3 * 5] = {};
            ngx2::framebuffer_t fb             = {ngx2::init_image_descr(3, 5, ngx2::IMAGE_FORMAT_RGB565), pixels};

            ngx2::draw_text(fb, &font, 1, 0, "A\nA", 0xffff, 1.0f);

            for (i32 y = 0; y < 5; ++y)
            {
                for (i32 x = 0; x < 3; ++x)
                    CHECK(pixels[x + y * 3] == (x == 1 && (y == 0 || y == 4) ? 0xdefb : 0));
            }
        }

        UNITTEST_TEST(text_invalid_inputs)
        {
            ngx2::color_t       pixels[2] = {0x1111, 0x2222};
            ngx2::framebuffer_t fb         = {ngx2::init_image_descr(2, 1, ngx2::IMAGE_FORMAT_RGB565), pixels};
            ngx2::font_t        font       = {};

            ngx2::draw_text(fb, nullptr, 0, 0, "A", 0xffff, 1.0f);
            ngx2::draw_text(fb, &font, 0, 0, nullptr, 0xffff, 1.0f);
            ngx2::draw_text(fb, &font, 0, 0, "", 0xffff, 1.0f);
            ngx2::draw_text(fb, &font, 0, 0, "A", 0xffff, 0.0f);
            ngx2::draw_text(fb, &font, 0, 0, "A", 0xffff, -1.0f);

            CHECK(pixels[0] == 0x1111);
            CHECK(pixels[1] == 0x2222);
        }
    }
}
UNITTEST_SUITE_END
