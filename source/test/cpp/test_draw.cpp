#include "ccore/c_allocator.h"

#include "cgx2/c_draw.h"

#include "cunittest/cunittest.h"

using namespace ncore;

namespace
{
    template <typename T>
    static void init_array(ngx2::array_t<T>& array, T const* data, u64 count)
    {
        array.m_offset = (i64)((byte const*)data - (byte const*)&array);
        array.m_size   = count;
    }

    static void init_bytes(ngx2::array_t<byte>& array, void const* data, u64 size)
    {
        init_array(array, (byte const*)data, size);
    }

    static void init_font(ngx2::font_t& font, u8 const* sdf, u64 sdf_size, i8 const* advance_x, ngx2::glyph_bearing_t const* bearings, ngx2::glyph_dimensions_t const* dimensions, u16 const* offsets, u64 glyph_count, u8* glyph_map)
    {
        init_array(font.m_data, sdf, sdf_size);
        init_array(font.m_glyphs_advance_x, advance_x, glyph_count);
        init_array(font.m_glyphs_bearing, bearings, glyph_count);
        init_array(font.m_glyphs_dimensions, dimensions, glyph_count);
        init_array(font.m_offsets, offsets, glyph_count);
        init_array(font.m_map, glyph_map, 128);
        font.m_font_type = 1;
    }
}

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
            ngx2::framebuffer_t fb             = {8, 8, pixels};
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
            init_bytes(rgb_sprite.pixel_data, rgb_pixels, sizeof(rgb_pixels));

            ngx2::color_t       framebuffer_pixels[] = {0x001f, 0x001f};
            ngx2::framebuffer_t fb                   = {2, 1, framebuffer_pixels};
            ngx2::rect_t        scissor              = {0, 0, 2, 1};
            ngx2::draw_sprite(fb, scissor, &rgb_sprite, nullptr, 0, 0, 0x07e0);
            CHECK(framebuffer_pixels[0] == 0xf800);
            CHECK(framebuffer_pixels[1] == 0x07e0);

            const u8            indices[]        = {1, 0};
            const ngx2::color_t palette_colors[] = {0xf800, 0x07e0};
            ngx2::sprite_t      indexed_sprite   = {};
            indexed_sprite.width                 = 2;
            indexed_sprite.height                = 1;
            indexed_sprite.pixel_format          = ngx2::FMT_PIXEL_I8;
            indexed_sprite.alpha_format          = ngx2::FMT_ALPHA_A4;
            init_bytes(indexed_sprite.pixel_data, indices, sizeof(indices));
            ngx2::palette_t palette = {ngx2::FMT_PALETTE_RGB565, {}};
            init_bytes(palette.data, palette_colors, sizeof(palette_colors));

            ngx2::draw_sprite(fb, scissor, &indexed_sprite, &palette, 0, 0, 0x07e0);
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
            init_bytes(sprite.pixel_data, source, sizeof(source));
            init_bytes(sprite.alpha_data, alpha, sizeof(alpha));

            ngx2::color_t       pixels[] = {0x0000, 0x0000, 0x0000, 0x0000};
            ngx2::framebuffer_t fb       = {2, 2, pixels};
            ngx2::rect_t        scissor  = {0, 0, 2, 2};
            ngx2::draw_sprite(fb, scissor, &sprite, nullptr, -1, 0, 0);

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
            init_bytes(sprite.pixel_data, source, sizeof(source));
            init_bytes(sprite.alpha_data, alpha, sizeof(alpha));

            ngx2::color_t       pixels[] = {0, 0, 0, 0};
            ngx2::framebuffer_t fb       = {4, 1, pixels};
            ngx2::rect_t        scissor  = {0, 0, 4, 1};
            ngx2::draw_sprite(fb, scissor, &sprite, nullptr, 0, 0, 0);

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
            init_bytes(sprite.pixel_data, source, sizeof(source));

            const u8 alpha_a4[] = {0x08};
            sprite.alpha_format = ngx2::FMT_ALPHA_A4;
            init_bytes(sprite.alpha_data, alpha_a4, sizeof(alpha_a4));
            ngx2::color_t       pixels_a4[] = {0, 0};
            ngx2::framebuffer_t fb_a4       = {2, 1, pixels_a4};
            ngx2::rect_t        scissor     = {0, 0, 2, 1};
            ngx2::draw_sprite(fb_a4, scissor, &sprite, nullptr, 0, 0, 0);
            CHECK(pixels_a4[0] == 0x0000);
            CHECK(pixels_a4[1] == 0x7bef);

            const u8 alpha_a8[] = {0, 128};
            sprite.alpha_format = ngx2::FMT_ALPHA_A8;
            init_bytes(sprite.alpha_data, alpha_a8, sizeof(alpha_a8));
            ngx2::color_t       pixels_a8[] = {0, 0};
            ngx2::framebuffer_t fb_a8       = {2, 1, pixels_a8};
            ngx2::draw_sprite(fb_a8, scissor, &sprite, nullptr, 0, 0, 0);
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
            init_bytes(sprite.pixel_data, source_a2, sizeof(source_a2));
            init_bytes(sprite.alpha_data, alpha_a2, sizeof(alpha_a2));

            ngx2::color_t       pixels_a2[] = {0, 0, 0, 0};
            ngx2::framebuffer_t fb_a2       = {4, 1, pixels_a2};
            ngx2::rect_t        scissor_a2  = {0, 0, 4, 1};
            ngx2::draw_sprite(fb_a2, scissor_a2, &sprite, nullptr, -1, 0, 0);
            CHECK(pixels_a2[0] == 0x39e7);
            CHECK(pixels_a2[1] == 0x7bef);
            CHECK(pixels_a2[2] == 0xffff);
            CHECK(pixels_a2[3] == 0xffff);

            const ngx2::color_t source_a4[] = {0xffff, 0xffff, 0xffff, 0xffff};
            const u8            alpha_a4[]  = {0x08, 0xf4};
            sprite.width                    = 4;
            sprite.alpha_format             = ngx2::FMT_ALPHA_A4;
            init_bytes(sprite.pixel_data, source_a4, sizeof(source_a4));
            init_bytes(sprite.alpha_data, alpha_a4, sizeof(alpha_a4));

            ngx2::color_t       pixels_a4[] = {0, 0, 0};
            ngx2::framebuffer_t fb_a4       = {3, 1, pixels_a4};
            ngx2::rect_t        scissor_a4  = {0, 0, 3, 1};
            ngx2::draw_sprite(fb_a4, scissor_a4, &sprite, nullptr, -1, 0, 0);
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
            init_bytes(sprite.pixel_data, indices, sizeof(indices));
            init_bytes(sprite.alpha_data, alpha, sizeof(alpha));
            ngx2::palette_t palette = {ngx2::FMT_PALETTE_RGB565, {}};
            init_bytes(palette.data, palette_colors, sizeof(palette_colors));

            ngx2::color_t       pixels[] = {0x001f, 0x001f};
            ngx2::framebuffer_t fb       = {2, 1, pixels};
            ngx2::rect_t        scissor  = {0, 0, 2, 1};
            ngx2::draw_sprite(fb, scissor, &sprite, &palette, 0, 0, 0);

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
            init_bytes(sprite.pixel_data, indices, sizeof(indices));
            ngx2::palette_t palette = {ngx2::FMT_PALETTE_RGB565, {}};
            init_bytes(palette.data, palette_colors, sizeof(palette_colors));
            ngx2::rect_t    scissor                = {0, 0, 2, 1};

            const u8 alpha_a2[] = {0x70};
            sprite.alpha_format = ngx2::FMT_ALPHA_A2;
            init_bytes(sprite.alpha_data, alpha_a2, sizeof(alpha_a2));
            ngx2::color_t       pixels_a2[] = {0, 0};
            ngx2::framebuffer_t fb_a2       = {2, 1, pixels_a2};
            ngx2::draw_sprite(fb_a2, scissor, &sprite, &palette, 0, 0, 0);
            CHECK(pixels_a2[0] == 0x39e7);
            CHECK(pixels_a2[1] == 0xffff);

            const u8 alpha_a4[] = {0x8f};
            sprite.alpha_format = ngx2::FMT_ALPHA_A4;
            init_bytes(sprite.alpha_data, alpha_a4, sizeof(alpha_a4));
            ngx2::color_t       pixels_a4[] = {0, 0};
            ngx2::framebuffer_t fb_a4       = {2, 1, pixels_a4};
            ngx2::draw_sprite(fb_a4, scissor, &sprite, &palette, 0, 0, 0);
            CHECK(pixels_a4[0] == 0x7bef);
            CHECK(pixels_a4[1] == 0xffff);

            const u8 alpha_a8[] = {128, 255};
            sprite.alpha_format = ngx2::FMT_ALPHA_A8;
            init_bytes(sprite.alpha_data, alpha_a8, sizeof(alpha_a8));
            ngx2::color_t       pixels_a8[] = {0, 0};
            ngx2::framebuffer_t fb_a8       = {2, 1, pixels_a8};
            ngx2::draw_sprite(fb_a8, scissor, &sprite, &palette, 0, 0, 0);
            CHECK(pixels_a8[0] == 0x8410);
            CHECK(pixels_a8[1] == 0xffff);
        }

        UNITTEST_TEST(text_metrics_and_sdf)
        {
            const u8                 sdf[]       = {0x18, 0xf0, 0xf0};
            u16                      offsets[]   = {0, 2};
            i8                       advance_x[] = {4, 1};
            ngx2::glyph_bearing_t    bearings[]  = {
                {1, 1},
                {0, 1},
            };
            ngx2::glyph_dimensions_t dimensions[] = {
                {3, 1},
                {1, 1},
            };
            u8           glyph_map[128];
            ngx2::font_t font = {};
            for (i32 i = 0; i < 128; ++i)
                glyph_map[i] = 0xff;
            glyph_map['A'] = 0;
            glyph_map['B'] = 1;
            init_font(font, sdf, sizeof(sdf), advance_x, bearings, dimensions, offsets, 2, glyph_map);

            ngx2::color_t       pixels[8 * 3] = {};
            ngx2::framebuffer_t fb             = {8, 3, pixels};

            ngx2::draw_text(fb, &font, 1, 2, "AB", 0xffff, 1.0f);

            CHECK(pixels[2 + 1 * 8] == 0x0000);
            CHECK(pixels[3 + 1 * 8] == 0x6b6d);
            CHECK(pixels[4 + 1 * 8] == 0xdefb);
            CHECK(pixels[5 + 1 * 8] == 0xdefb);
        }

        UNITTEST_TEST(text_mapping_and_advance)
        {
            const u8                 sdf[]       = {0xf0};
            u16                      offsets[]   = {0, 1};
            i8                       advance_x[] = {1, 2};
            ngx2::glyph_bearing_t    bearings[]  = {
                {0, 0},
                {0, 0},
            };
            ngx2::glyph_dimensions_t dimensions[] = {
                {1, 1},
                {0, 0},
            };
            u8           glyph_map[128];
            ngx2::font_t font = {};
            for (i32 i = 0; i < 128; ++i)
                glyph_map[i] = 0xff;
            glyph_map['A'] = 0;
            glyph_map['B'] = 1;
            init_font(font, sdf, sizeof(sdf), advance_x, bearings, dimensions, offsets, 2, glyph_map);

            ngx2::color_t       pixels[6] = {};
            ngx2::framebuffer_t fb         = {6, 1, pixels};
            const char          text[]     = {(char)0x80, 'B', 'A', 0};

            ngx2::draw_text(fb, &font, 1, 0, text, 0xffff, 1.0f);

            for (i32 x = 0; x < 6; ++x)
                CHECK(pixels[x] == (x == 3 ? 0xdefb : 0));
        }

        UNITTEST_TEST(text_packed_sdf)
        {
            const u8                 sdf[]       = {0x29, 0xaf};
            u16                      offsets[]   = {0};
            i8                       advance_x[] = {4};
            ngx2::glyph_bearing_t    bearings[]  = {
                {0, 0},
            };
            ngx2::glyph_dimensions_t dimensions[] = {
                {4, 1},
            };
            u8           glyph_map[128];
            ngx2::font_t font = {};
            for (i32 i = 0; i < 128; ++i)
                glyph_map[i] = 0xff;
            glyph_map['A'] = 0;
            init_font(font, sdf, sizeof(sdf), advance_x, bearings, dimensions, offsets, 1, glyph_map);

            ngx2::color_t       pixels[4] = {};
            ngx2::framebuffer_t fb         = {4, 1, pixels};

            ngx2::draw_text(fb, &font, 0, 0, "A", 0xffff, 1.0f);

            CHECK(pixels[0] == 0x0861);
            CHECK(pixels[1] == 0x7bef);
            CHECK(pixels[2] == 0x8c71);
            CHECK(pixels[3] == 0xdefb);
        }

        UNITTEST_TEST(text_sdf_offsets)
        {
            const u8                 sdf[]       = {0xf0, 0xf0, 0x2f};
            u16                      offsets[]   = {0, 2};
            i8                       advance_x[] = {4, 2};
            ngx2::glyph_bearing_t    bearings[]  = {
                {0, 0},
                {0, 0},
            };
            ngx2::glyph_dimensions_t dimensions[] = {
                {3, 1},
                {2, 1},
            };
            u8           glyph_map[128];
            ngx2::font_t font = {};
            for (i32 i = 0; i < 128; ++i)
                glyph_map[i] = 0xff;
            glyph_map['A'] = 0;
            glyph_map['B'] = 1;
            init_font(font, sdf, sizeof(sdf), advance_x, bearings, dimensions, offsets, 2, glyph_map);

            ngx2::color_t       pixels[6] = {};
            ngx2::framebuffer_t fb         = {6, 1, pixels};

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
            u16                      offsets[]   = {0};
            i8                       advance_x[] = {2};
            ngx2::glyph_bearing_t    bearings[]  = {
                {0, 0},
            };
            ngx2::glyph_dimensions_t dimensions[] = {
                {2, 1},
            };
            u8           glyph_map[128];
            ngx2::font_t font = {};
            for (i32 i = 0; i < 128; ++i)
                glyph_map[i] = 0xff;
            glyph_map['A'] = 0;
            init_font(font, sdf, sizeof(sdf), advance_x, bearings, dimensions, offsets, 1, glyph_map);

            ngx2::color_t       pixels[8 * 2] = {};
            ngx2::framebuffer_t fb             = {8, 2, pixels};

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
            u16                      offsets[]   = {0};
            i8                       advance_x[] = {2};
            ngx2::glyph_bearing_t    bearings[]  = {
                {0, 0},
            };
            ngx2::glyph_dimensions_t dimensions[] = {
                {1, 1},
            };
            u8           glyph_map[128];
            ngx2::font_t font = {};
            font.m_ascent                  = 2;
            font.m_descent                 = -1;
            font.m_line_gap                = 1;
            for (i32 i = 0; i < 128; ++i)
                glyph_map[i] = 0xff;
            glyph_map['A'] = 0;
            init_font(font, sdf, sizeof(sdf), advance_x, bearings, dimensions, offsets, 1, glyph_map);

            ngx2::color_t       pixels[3 * 5] = {};
            ngx2::framebuffer_t fb             = {3, 5, pixels};

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
            ngx2::framebuffer_t fb         = {2, 1, pixels};
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
