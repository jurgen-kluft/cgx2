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
            ngx2::framebuffer_t fb             = {ngx2::init_image_descr(8, 8, ngx2::FMT_PIXEL_RGB565), pixels};
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

        UNITTEST_TEST(sprite_opaque_paths)
        {
            const ngx2::color_t rgb_pixels[] = {0xf800, 0x07e0};
            ngx2::sprite_t      rgb_sprite   = {};
            rgb_sprite.width                 = 2;
            rgb_sprite.height                = 1;
            rgb_sprite.pixel_format          = ngx2::FMT_PIXEL_RGB565;
            rgb_sprite.alpha_format          = ngx2::FMT_ALPHA_A0;
            rgb_sprite.pixel_data            = (const byte*)rgb_pixels;

            ngx2::color_t       framebuffer_pixels[] = {0x001f, 0x001f};
            ngx2::framebuffer_t fb                   = {ngx2::init_image_descr(2, 1, ngx2::FMT_PIXEL_RGB565), framebuffer_pixels};
            ngx2::rect_t        scissor              = {0, 0, 2, 1};
            ngx2::draw_sprite(fb, scissor, &rgb_sprite, 0, 0);
            CHECK(framebuffer_pixels[0] == 0xf800);
            CHECK(framebuffer_pixels[1] == 0x07e0);

            const u8            indices[]        = {1, 0};
            const ngx2::color_t palette_colors[] = {0xf800, 0x07e0};
            ngx2::sprite_t      indexed_sprite   = {};
            indexed_sprite.width                 = 2;
            indexed_sprite.height                = 1;
            indexed_sprite.pixel_format          = ngx2::FMT_PIXEL_I8;
            indexed_sprite.alpha_format          = ngx2::FMT_ALPHA_A4;
            indexed_sprite.pixel_data            = indices;
            ngx2::palette_t palette               = {ngx2::FMT_PALETTE_RGB565, (u32)sizeof(palette_colors), (const byte*)palette_colors};

            ngx2::draw_sprite(fb, scissor, &indexed_sprite, &palette, 0, 0);
            CHECK(framebuffer_pixels[0] == 0x07e0);
            CHECK(framebuffer_pixels[1] == 0xf800);
        }

        UNITTEST_TEST(sprite_alpha_a1_rows_and_clipping)
        {
            const ngx2::color_t source[] = {0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff};
            const u8            alpha[]  = {0xa0, 0x40};
            ngx2::sprite_t      sprite   = {};
            sprite.width                 = 3;
            sprite.height                = 2;
            sprite.pixel_format          = ngx2::FMT_PIXEL_RGB565;
            sprite.alpha_format          = ngx2::FMT_ALPHA_A1;
            sprite.pixel_data            = (const byte*)source;
            sprite.alpha_data            = alpha;

            ngx2::color_t       pixels[] = {0x0000, 0x0000, 0x0000, 0x0000};
            ngx2::framebuffer_t fb       = {ngx2::init_image_descr(2, 2, ngx2::FMT_PIXEL_RGB565), pixels};
            ngx2::rect_t        scissor  = {0, 0, 2, 2};
            ngx2::draw_sprite(fb, scissor, &sprite, -1, 0);

            CHECK(pixels[0] == 0x0000);
            CHECK(pixels[1] == 0xffff);
            CHECK(pixels[2] == 0xffff);
            CHECK(pixels[3] == 0x0000);
        }

        UNITTEST_TEST(sprite_alpha_a2)
        {
            const ngx2::color_t source[] = {0xffff, 0xffff, 0xffff, 0xffff};
            const u8            alpha[]  = {0x1b};
            ngx2::sprite_t      sprite   = {};
            sprite.width                 = 4;
            sprite.height                = 1;
            sprite.pixel_format          = ngx2::FMT_PIXEL_RGB565;
            sprite.alpha_format          = ngx2::FMT_ALPHA_A2;
            sprite.pixel_data            = (const byte*)source;
            sprite.alpha_data            = alpha;

            ngx2::color_t       pixels[] = {0, 0, 0, 0};
            ngx2::framebuffer_t fb       = {ngx2::init_image_descr(4, 1, ngx2::FMT_PIXEL_RGB565), pixels};
            ngx2::rect_t        scissor  = {0, 0, 4, 1};
            ngx2::draw_sprite(fb, scissor, &sprite, 0, 0);

            CHECK(pixels[0] == 0x0000);
            CHECK(pixels[1] == 0x39e7);
            CHECK(pixels[2] == 0x7bef);
            CHECK(pixels[3] == 0xffff);
        }

        UNITTEST_TEST(sprite_alpha_a4_a8)
        {
            const ngx2::color_t source[] = {0xffff, 0xffff};
            ngx2::sprite_t      sprite   = {};
            sprite.width                 = 2;
            sprite.height                = 1;
            sprite.pixel_format          = ngx2::FMT_PIXEL_RGB565;
            sprite.pixel_data            = (const byte*)source;

            const u8 alpha_a4[] = {0x08};
            sprite.alpha_format = ngx2::FMT_ALPHA_A4;
            sprite.alpha_data   = alpha_a4;
            ngx2::color_t       pixels_a4[] = {0, 0};
            ngx2::framebuffer_t fb_a4       = {ngx2::init_image_descr(2, 1, ngx2::FMT_PIXEL_RGB565), pixels_a4};
            ngx2::rect_t        scissor     = {0, 0, 2, 1};
            ngx2::draw_sprite(fb_a4, scissor, &sprite, 0, 0);
            CHECK(pixels_a4[0] == 0x0000);
            CHECK(pixels_a4[1] == 0x7bef);

            const u8 alpha_a8[] = {0, 128};
            sprite.alpha_format = ngx2::FMT_ALPHA_A8;
            sprite.alpha_data   = alpha_a8;
            ngx2::color_t       pixels_a8[] = {0, 0};
            ngx2::framebuffer_t fb_a8       = {ngx2::init_image_descr(2, 1, ngx2::FMT_PIXEL_RGB565), pixels_a8};
            ngx2::draw_sprite(fb_a8, scissor, &sprite, 0, 0);
            CHECK(pixels_a8[0] == 0x0000);
            CHECK(pixels_a8[1] == 0x8410);
        }

        UNITTEST_TEST(sprite_alpha_packed_clipping)
        {
            const ngx2::color_t source_a2[] = {0xffff, 0xffff, 0xffff, 0xffff, 0xffff};
            const u8            alpha_a2[]  = {0x1b, 0xc0};
            ngx2::sprite_t      sprite      = {};
            sprite.width                    = 5;
            sprite.height                   = 1;
            sprite.pixel_format             = ngx2::FMT_PIXEL_RGB565;
            sprite.alpha_format             = ngx2::FMT_ALPHA_A2;
            sprite.pixel_data               = (const byte*)source_a2;
            sprite.alpha_data               = alpha_a2;

            ngx2::color_t       pixels_a2[] = {0, 0, 0, 0};
            ngx2::framebuffer_t fb_a2       = {ngx2::init_image_descr(4, 1, ngx2::FMT_PIXEL_RGB565), pixels_a2};
            ngx2::rect_t        scissor_a2  = {0, 0, 4, 1};
            ngx2::draw_sprite(fb_a2, scissor_a2, &sprite, -1, 0);
            CHECK(pixels_a2[0] == 0x39e7);
            CHECK(pixels_a2[1] == 0x7bef);
            CHECK(pixels_a2[2] == 0xffff);
            CHECK(pixels_a2[3] == 0xffff);

            const ngx2::color_t source_a4[] = {0xffff, 0xffff, 0xffff, 0xffff};
            const u8            alpha_a4[]  = {0x08, 0xf4};
            sprite.width                    = 4;
            sprite.alpha_format             = ngx2::FMT_ALPHA_A4;
            sprite.pixel_data               = (const byte*)source_a4;
            sprite.alpha_data               = alpha_a4;

            ngx2::color_t       pixels_a4[] = {0, 0, 0};
            ngx2::framebuffer_t fb_a4       = {ngx2::init_image_descr(3, 1, ngx2::FMT_PIXEL_RGB565), pixels_a4};
            ngx2::rect_t        scissor_a4  = {0, 0, 3, 1};
            ngx2::draw_sprite(fb_a4, scissor_a4, &sprite, -1, 0);
            CHECK(pixels_a4[0] == 0x7bef);
            CHECK(pixels_a4[1] == 0xffff);
            CHECK(pixels_a4[2] == 0x39e7);
        }

        UNITTEST_TEST(sprite_indexed_alpha)
        {
            const u8            indices[]        = {0, 1};
            const u8            alpha[]          = {0x40};
            const ngx2::color_t palette_colors[] = {0xf800, 0x07e0};
            ngx2::sprite_t      sprite            = {};
            sprite.width                          = 2;
            sprite.height                         = 1;
            sprite.pixel_format                   = ngx2::FMT_PIXEL_I8;
            sprite.alpha_format                   = ngx2::FMT_ALPHA_A1;
            sprite.pixel_data                     = indices;
            sprite.alpha_data                     = alpha;
            ngx2::palette_t palette                = {ngx2::FMT_PALETTE_RGB565, (u32)sizeof(palette_colors), (const byte*)palette_colors};

            ngx2::color_t       pixels[] = {0x001f, 0x001f};
            ngx2::framebuffer_t fb       = {ngx2::init_image_descr(2, 1, ngx2::FMT_PIXEL_RGB565), pixels};
            ngx2::rect_t        scissor  = {0, 0, 2, 1};
            ngx2::draw_sprite(fb, scissor, &sprite, &palette, 0, 0);

            CHECK(pixels[0] == 0x001f);
            CHECK(pixels[1] == 0x07e0);
        }

        UNITTEST_TEST(sprite_indexed_alpha_formats)
        {
            const u8            indices[]        = {0, 0};
            const ngx2::color_t palette_colors[] = {0xffff};
            ngx2::sprite_t      sprite            = {};
            sprite.width                          = 2;
            sprite.height                         = 1;
            sprite.pixel_format                   = ngx2::FMT_PIXEL_I8;
            sprite.pixel_data                     = indices;
            ngx2::palette_t palette                = {ngx2::FMT_PALETTE_RGB565, (u32)sizeof(palette_colors), (const byte*)palette_colors};
            ngx2::rect_t    scissor                = {0, 0, 2, 1};

            const u8 alpha_a2[] = {0x70};
            sprite.alpha_format = ngx2::FMT_ALPHA_A2;
            sprite.alpha_data   = alpha_a2;
            ngx2::color_t       pixels_a2[] = {0, 0};
            ngx2::framebuffer_t fb_a2       = {ngx2::init_image_descr(2, 1, ngx2::FMT_PIXEL_RGB565), pixels_a2};
            ngx2::draw_sprite(fb_a2, scissor, &sprite, &palette, 0, 0);
            CHECK(pixels_a2[0] == 0x39e7);
            CHECK(pixels_a2[1] == 0xffff);

            const u8 alpha_a4[] = {0x8f};
            sprite.alpha_format = ngx2::FMT_ALPHA_A4;
            sprite.alpha_data   = alpha_a4;
            ngx2::color_t       pixels_a4[] = {0, 0};
            ngx2::framebuffer_t fb_a4       = {ngx2::init_image_descr(2, 1, ngx2::FMT_PIXEL_RGB565), pixels_a4};
            ngx2::draw_sprite(fb_a4, scissor, &sprite, &palette, 0, 0);
            CHECK(pixels_a4[0] == 0x7bef);
            CHECK(pixels_a4[1] == 0xffff);

            const u8 alpha_a8[] = {128, 255};
            sprite.alpha_format = ngx2::FMT_ALPHA_A8;
            sprite.alpha_data   = alpha_a8;
            ngx2::color_t       pixels_a8[] = {0, 0};
            ngx2::framebuffer_t fb_a8       = {ngx2::init_image_descr(2, 1, ngx2::FMT_PIXEL_RGB565), pixels_a8};
            ngx2::draw_sprite(fb_a8, scissor, &sprite, &palette, 0, 0);
            CHECK(pixels_a8[0] == 0x8410);
            CHECK(pixels_a8[1] == 0xffff);
        }

        UNITTEST_TEST(text_metrics_and_sdf)
        {
            const u8                 sdf[]       = {0x18, 0xf0, 0xf0};
            u32                      offsets[]   = {0, 2};
            i8                       advance_x[] = {4, 1};
            ngx2::glyph_bearing_t    bearings[]  = {
                {1, 1},
                {0, 1},
            };
            ngx2::glyph_dimensions_t dimensions[] = {
                {3, 1},
                {1, 1},
            };
            ngx2::font_t font              = {};
            font.m_sdf                     = sdf;
            font.m_glyphs_advance_x_size   = (u32)sizeof(advance_x);
            font.m_glyphs_advance_x        = advance_x;
            font.m_glyphs_bearing_size     = (u32)sizeof(bearings);
            font.m_glyphs_bearing          = bearings;
            font.m_glyphs_dimensions_size  = (u32)sizeof(dimensions);
            font.m_glyphs_dimensions       = dimensions;
            font.m_offsets                 = offsets;
            for (i32 i = 0; i < (i32)sizeof(font.m_map); ++i)
                font.m_map[i] = 0xff;
            font.m_map['A'] = 0;
            font.m_map['B'] = 1;

            ngx2::color_t       pixels[8 * 3] = {};
            ngx2::framebuffer_t fb             = {ngx2::init_image_descr(8, 3, ngx2::FMT_PIXEL_RGB565), pixels};

            ngx2::draw_text(fb, &font, 1, 2, "AB", 0xffff, 1.0f);

            CHECK(pixels[2 + 1 * 8] == 0x0000);
            CHECK(pixels[3 + 1 * 8] == 0x6b6d);
            CHECK(pixels[4 + 1 * 8] == 0xdefb);
            CHECK(pixels[5 + 1 * 8] == 0xdefb);
        }

        UNITTEST_TEST(text_mapping_and_advance)
        {
            const u8                 sdf[]       = {0xf0};
            u32                      offsets[]   = {0, 1};
            i8                       advance_x[] = {1, 2};
            ngx2::glyph_bearing_t    bearings[]  = {
                {0, 0},
                {0, 0},
            };
            ngx2::glyph_dimensions_t dimensions[] = {
                {1, 1},
                {0, 0},
            };
            ngx2::font_t font              = {};
            font.m_sdf                     = sdf;
            font.m_glyphs_advance_x_size   = (u32)sizeof(advance_x);
            font.m_glyphs_advance_x        = advance_x;
            font.m_glyphs_bearing_size     = (u32)sizeof(bearings);
            font.m_glyphs_bearing          = bearings;
            font.m_glyphs_dimensions_size  = (u32)sizeof(dimensions);
            font.m_glyphs_dimensions       = dimensions;
            font.m_offsets                 = offsets;
            for (i32 i = 0; i < (i32)sizeof(font.m_map); ++i)
                font.m_map[i] = 0xff;
            font.m_map['A'] = 0;
            font.m_map['B'] = 1;

            ngx2::color_t       pixels[6] = {};
            ngx2::framebuffer_t fb         = {ngx2::init_image_descr(6, 1, ngx2::FMT_PIXEL_RGB565), pixels};
            const char          text[]     = {(char)0x80, 'B', 'A', 0};

            ngx2::draw_text(fb, &font, 1, 0, text, 0xffff, 1.0f);

            for (i32 x = 0; x < 6; ++x)
                CHECK(pixels[x] == (x == 3 ? 0xdefb : 0));
        }

        UNITTEST_TEST(text_packed_sdf)
        {
            const u8                 sdf[]       = {0x29, 0xaf};
            u32                      offsets[]   = {0};
            i8                       advance_x[] = {4};
            ngx2::glyph_bearing_t    bearings[]  = {
                {0, 0},
            };
            ngx2::glyph_dimensions_t dimensions[] = {
                {4, 1},
            };
            ngx2::font_t font              = {};
            font.m_sdf                     = sdf;
            font.m_glyphs_advance_x_size   = (u32)sizeof(advance_x);
            font.m_glyphs_advance_x        = advance_x;
            font.m_glyphs_bearing_size     = (u32)sizeof(bearings);
            font.m_glyphs_bearing          = bearings;
            font.m_glyphs_dimensions_size  = (u32)sizeof(dimensions);
            font.m_glyphs_dimensions       = dimensions;
            font.m_offsets                 = offsets;
            for (i32 i = 0; i < (i32)sizeof(font.m_map); ++i)
                font.m_map[i] = 0xff;
            font.m_map['A'] = 0;

            ngx2::color_t       pixels[4] = {};
            ngx2::framebuffer_t fb         = {ngx2::init_image_descr(4, 1, ngx2::FMT_PIXEL_RGB565), pixels};

            ngx2::draw_text(fb, &font, 0, 0, "A", 0xffff, 1.0f);

            CHECK(pixels[0] == 0x0861);
            CHECK(pixels[1] == 0x7bef);
            CHECK(pixels[2] == 0x8c71);
            CHECK(pixels[3] == 0xdefb);
        }

        UNITTEST_TEST(text_sdf_offsets)
        {
            const u8                 sdf[]       = {0xf0, 0xf0, 0x2f};
            u32                      offsets[]   = {0, 2};
            i8                       advance_x[] = {4, 2};
            ngx2::glyph_bearing_t    bearings[]  = {
                {0, 0},
                {0, 0},
            };
            ngx2::glyph_dimensions_t dimensions[] = {
                {3, 1},
                {2, 1},
            };
            ngx2::font_t font              = {};
            font.m_sdf                     = sdf;
            font.m_glyphs_advance_x_size   = (u32)sizeof(advance_x);
            font.m_glyphs_advance_x        = advance_x;
            font.m_glyphs_bearing_size     = (u32)sizeof(bearings);
            font.m_glyphs_bearing          = bearings;
            font.m_glyphs_dimensions_size  = (u32)sizeof(dimensions);
            font.m_glyphs_dimensions       = dimensions;
            font.m_offsets                 = offsets;
            for (i32 i = 0; i < (i32)sizeof(font.m_map); ++i)
                font.m_map[i] = 0xff;
            font.m_map['A'] = 0;
            font.m_map['B'] = 1;

            ngx2::color_t       pixels[6] = {};
            ngx2::framebuffer_t fb         = {ngx2::init_image_descr(6, 1, ngx2::FMT_PIXEL_RGB565), pixels};

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
            const u8                 sdf[]       = {0x2f};
            u32                      offsets[]   = {0};
            i8                       advance_x[] = {2};
            ngx2::glyph_bearing_t    bearings[]  = {
                {0, 0},
            };
            ngx2::glyph_dimensions_t dimensions[] = {
                {2, 1},
            };
            ngx2::font_t font              = {};
            font.m_sdf                     = sdf;
            font.m_glyphs_advance_x_size   = (u32)sizeof(advance_x);
            font.m_glyphs_advance_x        = advance_x;
            font.m_glyphs_bearing_size     = (u32)sizeof(bearings);
            font.m_glyphs_bearing          = bearings;
            font.m_glyphs_dimensions_size  = (u32)sizeof(dimensions);
            font.m_glyphs_dimensions       = dimensions;
            font.m_offsets                 = offsets;
            for (i32 i = 0; i < (i32)sizeof(font.m_map); ++i)
                font.m_map[i] = 0xff;
            font.m_map['A'] = 0;

            ngx2::color_t       pixels[8 * 2] = {};
            ngx2::framebuffer_t fb             = {ngx2::init_image_descr(8, 2, ngx2::FMT_PIXEL_RGB565), pixels};

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
            const u8                 sdf[]       = {0xf0};
            u32                      offsets[]   = {0};
            i8                       advance_x[] = {2};
            ngx2::glyph_bearing_t    bearings[]  = {
                {0, 0},
            };
            ngx2::glyph_dimensions_t dimensions[] = {
                {1, 1},
            };
            ngx2::font_t font              = {};
            font.m_sdf                     = sdf;
            font.m_glyphs_advance_x_size   = (u32)sizeof(advance_x);
            font.m_glyphs_advance_x        = advance_x;
            font.m_glyphs_bearing_size     = (u32)sizeof(bearings);
            font.m_glyphs_bearing          = bearings;
            font.m_glyphs_dimensions_size  = (u32)sizeof(dimensions);
            font.m_glyphs_dimensions       = dimensions;
            font.m_offsets                 = offsets;
            font.m_ascent                  = 2;
            font.m_descent                 = -1;
            font.m_line_gap                = 1;
            for (i32 i = 0; i < (i32)sizeof(font.m_map); ++i)
                font.m_map[i] = 0xff;
            font.m_map['A'] = 0;

            ngx2::color_t       pixels[3 * 5] = {};
            ngx2::framebuffer_t fb             = {ngx2::init_image_descr(3, 5, ngx2::FMT_PIXEL_RGB565), pixels};

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
            ngx2::framebuffer_t fb         = {ngx2::init_image_descr(2, 1, ngx2::FMT_PIXEL_RGB565), pixels};
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
