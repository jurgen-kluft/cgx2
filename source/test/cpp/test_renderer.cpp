#include "cgx2/c_renderer.h"

#include "cunittest/cunittest.h"

using namespace ncore;

namespace
{
    static const u16 c_clear_color = 0x1357;

    static void clear_pixels(u16* pixels, u32 count)
    {
        for (u32 index = 0; index < count; ++index)
            pixels[index] = c_clear_color;
    }

    template <typename T>
    static void set_array(ngx2::array_t<T>& array, T* data, u32 count)
    {
        array.m_offset = (i32)((u8*)data - (u8*)&array);
        array.m_size   = count;
    }
}

UNITTEST_SUITE_BEGIN(gx2)
{
    UNITTEST_FIXTURE(renderer)
    {
        UNITTEST_FIXTURE_SETUP() {}
        UNITTEST_FIXTURE_TEARDOWN() {}

        UNITTEST_TEST(hline)
        {
            u16 pixels[20];
            u16 expected[20];
            clear_pixels(pixels, 20);
            clear_pixels(expected, 20);

            ngx2::ctx_t ctx = {pixels, 5, 4, 10};
            ngx2::draw_hline(ctx, 3, -2, 11, 0xFFFFFF);

            expected[5] = 0xFFFF;
            expected[6] = 0xFFFF;
            expected[7] = 0xFFFF;
            expected[8] = 0xFFFF;
            for (u32 index = 0; index < 20; ++index)
                CHECK_EQUAL(expected[index], pixels[index]);

            ngx2::draw_hline(ctx, 0, 4, 9, 0xFFFFFF);
            for (u32 index = 0; index < 20; ++index)
                CHECK_EQUAL(expected[index], pixels[index]);
        }

        UNITTEST_TEST(vline)
        {
            u16 pixels[20];
            u16 expected[20];
            clear_pixels(pixels, 20);
            clear_pixels(expected, 20);

            ngx2::ctx_t ctx = {pixels, 5, 4, 10};
            ngx2::draw_vline(ctx, 2, 14, 9, 0xFFFFFF);

            expected[2]  = 0xFFFF;
            expected[7]  = 0xFFFF;
            expected[12] = 0xFFFF;
            expected[17] = 0xFFFF;
            for (u32 index = 0; index < 20; ++index)
                CHECK_EQUAL(expected[index], pixels[index]);

            ngx2::draw_vline(ctx, -1, 10, 13, 0xFFFFFF);
            for (u32 index = 0; index < 20; ++index)
                CHECK_EQUAL(expected[index], pixels[index]);
        }

        UNITTEST_TEST(hdline)
        {
            u16 pixels[20];
            u16 expected[20];
            clear_pixels(pixels, 20);
            clear_pixels(expected, 20);

            ngx2::ctx_t ctx = {pixels, 5, 4, 10};
            ngx2::draw_hdline(ctx, -2, 5, 11, 2, 1, 0xFFFFFF);

            expected[6] = 0xFFFF;
            expected[7] = 0xFFFF;
            expected[9] = 0xFFFF;
            for (u32 index = 0; index < 20; ++index)
                CHECK_EQUAL(expected[index], pixels[index]);

            ngx2::draw_hdline(ctx, 4, 0, 12, 0, 0, 0xFFFFFF);
            for (u32 index = 0; index < 20; ++index)
                CHECK_EQUAL(expected[index], pixels[index]);
        }

        UNITTEST_TEST(vdline)
        {
            u16 pixels[20];
            u16 expected[20];
            clear_pixels(pixels, 20);
            clear_pixels(expected, 20);

            ngx2::ctx_t ctx = {pixels, 5, 4, 10};
            ngx2::draw_vdline(ctx, 2, 14, 9, 1, 1, 0xFFFFFF);

            expected[7]  = 0xFFFF;
            expected[17] = 0xFFFF;
            for (u32 index = 0; index < 20; ++index)
                CHECK_EQUAL(expected[index], pixels[index]);

            ngx2::draw_vdline(ctx, 3, 10, 13, 0, 2, 0xFFFFFF);
            for (u32 index = 0; index < 20; ++index)
                CHECK_EQUAL(expected[index], pixels[index]);
        }

        UNITTEST_TEST(sprite_rgb565_a0)
        {
            u16 pixels[20];
            u16 expected[20];
            u16 source[] = {0xF800, 0x07E0, 0x001F, 0xFFFF};
            ngx2::sprite_t sprite = {};
            sprite.m_width        = 2;
            sprite.m_height       = 2;
            sprite.m_pixel_format = ngx2::FMT_PIXEL_RGB565;
            sprite.m_alpha_format = ngx2::FMT_ALPHA_A0;
            set_array(sprite.m_pixel_data, (byte*)source, sizeof(source));

            clear_pixels(pixels, 20);
            clear_pixels(expected, 20);
            ngx2::ctx_t ctx = {pixels, 5, 4, 10};
            ngx2::draw_sprite(ctx, &sprite, nullptr, 1, 11, 0);

            expected[6]  = 0xF800;
            expected[7]  = 0x07E0;
            expected[11] = 0x001F;
            expected[12] = 0xFFFF;
            for (u32 index = 0; index < 20; ++index)
                CHECK_EQUAL(expected[index], pixels[index]);

            clear_pixels(pixels, 20);
            clear_pixels(expected, 20);
            ngx2::draw_sprite(ctx, &sprite, nullptr, -1, 12, 0);
            expected[10] = 0x07E0;
            expected[15] = 0xFFFF;
            for (u32 index = 0; index < 20; ++index)
                CHECK_EQUAL(expected[index], pixels[index]);
        }

        UNITTEST_TEST(sprite_i8_a0)
        {
            u16 pixels[20];
            u16 expected[20];
            u8  source[]  = {1, 0};
            u16 colors[]  = {0x07E0, 0xF800};
            ngx2::sprite_t sprite  = {};
            ngx2::palette_t palette = {};
            sprite.m_width          = 2;
            sprite.m_height         = 1;
            sprite.m_pixel_format   = ngx2::FMT_PIXEL_I8;
            sprite.m_alpha_format   = ngx2::FMT_ALPHA_A0;
            set_array(sprite.m_pixel_data, source, sizeof(source));
            set_array(palette.m_data, (byte*)colors, sizeof(colors));

            clear_pixels(pixels, 20);
            clear_pixels(expected, 20);
            ngx2::ctx_t ctx = {pixels, 5, 4, 10};
            ngx2::draw_sprite(ctx, &sprite, &palette, 2, 12, 0);

            expected[12] = 0xF800;
            expected[13] = 0x07E0;
            for (u32 index = 0; index < 20; ++index)
                CHECK_EQUAL(expected[index], pixels[index]);
        }

        UNITTEST_TEST(sprite_scaled_i8_a0)
        {
            u16 pixels[20];
            u16 expected[20];
            u8  source[] = {1, 0};
            u16 colors[] = {0x07E0, 0xF800};
            ngx2::sprite_t sprite = {};
            ngx2::palette_t palette = {};
            sprite.m_width          = 2;
            sprite.m_height         = 1;
            sprite.m_pixel_format   = ngx2::FMT_PIXEL_I8;
            sprite.m_alpha_format   = ngx2::FMT_ALPHA_A0;
            set_array(sprite.m_pixel_data, source, sizeof(source));
            set_array(palette.m_data, (byte*)colors, sizeof(colors));

            clear_pixels(pixels, 20);
            clear_pixels(expected, 20);
            ngx2::ctx_t ctx = {pixels, 5, 4, 10};
            ngx2::draw_sprite(ctx, &sprite, &palette, 0, 11, 4, 2, 0);

            expected[5]  = 0xF800;
            expected[6]  = 0xF800;
            expected[7]  = 0x07E0;
            expected[8]  = 0x07E0;
            expected[10] = 0xF800;
            expected[11] = 0xF800;
            expected[12] = 0x07E0;
            expected[13] = 0x07E0;
            for (u32 index = 0; index < 20; ++index)
                CHECK_EQUAL(expected[index], pixels[index]);
        }

        UNITTEST_TEST(sprite_alpha_only)
        {
            u16 pixels[20];
            u16 expected[20];
            u8  alpha[] = {0x80};
            ngx2::sprite_t sprite = {};
            sprite.m_width        = 2;
            sprite.m_height       = 1;
            sprite.m_alpha_format = ngx2::FMT_ALPHA_A1;
            set_array(sprite.m_alpha_data, alpha, sizeof(alpha));

            clear_pixels(pixels, 20);
            clear_pixels(expected, 20);
            ngx2::ctx_t ctx = {pixels, 5, 4, 10};
            ngx2::draw_sprite(ctx, &sprite, nullptr, 1, 11, 0xFF0000);

            expected[6] = 0xF800;
            for (u32 index = 0; index < 20; ++index)
                CHECK_EQUAL(expected[index], pixels[index]);

            sprite.m_width = 1;
            sprite.m_alpha_format = ngx2::FMT_ALPHA_A2;
            alpha[0] = 0xC0;
            clear_pixels(pixels, 20);
            clear_pixels(expected, 20);
            ngx2::draw_sprite(ctx, &sprite, nullptr, 3, 12, 0xFF0000);
            expected[13] = 0xF800;
            for (u32 index = 0; index < 20; ++index)
                CHECK_EQUAL(expected[index], pixels[index]);

            sprite.m_alpha_format = ngx2::FMT_ALPHA_A4;
            alpha[0] = 0xF0;
            clear_pixels(pixels, 20);
            clear_pixels(expected, 20);
            ngx2::draw_sprite(ctx, &sprite, nullptr, 4, 13, 0xFF0000);
            expected[19] = 0xF800;
            for (u32 index = 0; index < 20; ++index)
                CHECK_EQUAL(expected[index], pixels[index]);

            sprite.m_alpha_format = ngx2::FMT_ALPHA_A8;
            alpha[0] = 0xFF;
            clear_pixels(pixels, 20);
            clear_pixels(expected, 20);
            ngx2::draw_sprite(ctx, &sprite, nullptr, 0, 10, 0xFF0000);
            expected[0] = 0xF800;
            for (u32 index = 0; index < 20; ++index)
                CHECK_EQUAL(expected[index], pixels[index]);
        }

        UNITTEST_TEST(sprite_packed_alpha_rows)
        {
            u16 pixels[20];
            u16 expected[20];
            u16 source[] = {0xF800, 0x07E0, 0x001F, 0xFFFF};
            u8  alpha[]  = {0x80, 0x40, 0x00};
            ngx2::sprite_t sprite = {};
            sprite.m_width        = 2;
            sprite.m_height       = 2;
            sprite.m_pixel_format = ngx2::FMT_PIXEL_RGB565;
            sprite.m_alpha_format = ngx2::FMT_ALPHA_A1;
            set_array(sprite.m_pixel_data, (byte*)source, sizeof(source));
            set_array(sprite.m_alpha_data, alpha, 2);

            clear_pixels(pixels, 20);
            clear_pixels(expected, 20);
            ngx2::ctx_t ctx = {pixels, 5, 4, 10};
            ngx2::draw_sprite(ctx, &sprite, nullptr, 1, 11, 0);

            expected[6]  = 0xF800;
            expected[12] = 0xFFFF;
            for (u32 index = 0; index < 20; ++index)
                CHECK_EQUAL(expected[index], pixels[index]);
        }

        UNITTEST_TEST(sprite_scaled)
        {
            u16 pixels[20];
            u16 expected[20];
            u16 source[] = {0xF800, 0x07E0, 0x001F, 0xFFFF};
            ngx2::sprite_t sprite = {};
            sprite.m_width        = 2;
            sprite.m_height       = 2;
            sprite.m_pixel_format = ngx2::FMT_PIXEL_RGB565;
            sprite.m_alpha_format = ngx2::FMT_ALPHA_A0;
            set_array(sprite.m_pixel_data, (byte*)source, sizeof(source));

            clear_pixels(pixels, 20);
            clear_pixels(expected, 20);
            ngx2::ctx_t ctx = {pixels, 5, 4, 10};
            ngx2::draw_sprite(ctx, &sprite, nullptr, 0, 10, 4, 4, 0);

            expected[0]  = 0xF800;
            expected[1]  = 0xF800;
            expected[2]  = 0x07E0;
            expected[3]  = 0x07E0;
            expected[5]  = 0xF800;
            expected[6]  = 0xF800;
            expected[7]  = 0x07E0;
            expected[8]  = 0x07E0;
            expected[10] = 0x001F;
            expected[11] = 0x001F;
            expected[12] = 0xFFFF;
            expected[13] = 0xFFFF;
            expected[15] = 0x001F;
            expected[16] = 0x001F;
            expected[17] = 0xFFFF;
            expected[18] = 0xFFFF;
            for (u32 index = 0; index < 20; ++index)
                CHECK_EQUAL(expected[index], pixels[index]);

            u8 alpha[] = {0x80};
            sprite = {};
            sprite.m_width        = 2;
            sprite.m_height       = 1;
            sprite.m_alpha_format = ngx2::FMT_ALPHA_A1;
            set_array(sprite.m_alpha_data, alpha, sizeof(alpha));
            clear_pixels(pixels, 20);
            clear_pixels(expected, 20);
            ngx2::draw_sprite(ctx, &sprite, nullptr, 0, 11, 4, 2, 0xFF0000);
            expected[5]  = 0xF800;
            expected[6]  = 0xF800;
            expected[10] = 0xF800;
            expected[11] = 0xF800;
            for (u32 index = 0; index < 20; ++index)
                CHECK_EQUAL(expected[index], pixels[index]);
        }

        UNITTEST_TEST(text)
        {
            u16                     pixels[20];
            u16                     expected[20];
            u8                      data[]    = {0, 0, 0, 0, 0, 0, 0, 0, 0xF1};
            i8                      advance[] = {3};
            ngx2::glyph_bearing_t   bearing[] = {{0, 0}};
            ngx2::glyph_dimensions_t dimensions[] = {{2, 1}};
            u16                     offsets[] = {1};
            u8                      map[128];
            ngx2::font_t            font = {};
            for (u32 index = 0; index < 128; ++index)
                map[index] = 0xFF;
            map['A'] = 0;
            set_array(font.m_data, data, sizeof(data));
            set_array(font.m_glyphs_advance_x, advance, 1);
            set_array(font.m_glyphs_bearing, bearing, 1);
            set_array(font.m_glyphs_dimensions, dimensions, 1);
            set_array(font.m_offsets, offsets, 1);
            set_array(font.m_map, map, 128);
            font.m_ascent  = 16;
            font.m_descent = 0;

            clear_pixels(pixels, 20);
            clear_pixels(expected, 20);
            ngx2::ctx_t ctx = {pixels, 5, 4, 10};
            ngx2::draw_text(ctx, &font, 16, 1, 11, "A", 0xFFFFFF);

            expected[6] = 0xDF5E;
            for (u32 index = 0; index < 20; ++index)
                CHECK_EQUAL(expected[index], pixels[index]);
        }

        UNITTEST_TEST(text_layout)
        {
            u16                     pixels[100];
            u16                     expected[100];
            u8                      data[]    = {0xF0};
            i8                      advance[] = {3};
            ngx2::glyph_bearing_t   bearing[] = {{0, 0}};
            ngx2::glyph_dimensions_t dimensions[] = {{1, 1}};
            u16                     offsets[] = {0};
            u8                      map[128];
            ngx2::font_t            font = {};
            for (u32 index = 0; index < 128; ++index)
                map[index] = 0xFF;
            map['A'] = 0;
            set_array(font.m_data, data, sizeof(data));
            set_array(font.m_glyphs_advance_x, advance, 1);
            set_array(font.m_glyphs_bearing, bearing, 1);
            set_array(font.m_glyphs_dimensions, dimensions, 1);
            set_array(font.m_offsets, offsets, 1);
            set_array(font.m_map, map, 128);
            font.m_ascent  = 9;
            font.m_descent = 0;

            clear_pixels(pixels, 100);
            clear_pixels(expected, 100);
            ngx2::ctx_t ctx = {pixels, 5, 20, 0};
            ngx2::draw_text(ctx, &font, 9, 0, 0, "A?\nA\rA", 0xFFFFFF);

            expected[0]  = 0xDF5E;
            expected[45] = 0xDF5E;
            expected[48] = 0xDF5E;
            for (u32 index = 0; index < 100; ++index)
                CHECK_EQUAL(expected[index], pixels[index]);
        }
    }
}
UNITTEST_SUITE_END
