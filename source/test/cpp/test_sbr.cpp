#include "cgx2/c_slice_based_renderer.h"

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

    static void init_palette(ngx2::palette_t& palette, ngx2::color_t const* colors, u64 count)
    {
        palette.format = ngx2::FMT_PALETTE_RGB565;
        init_bytes(palette.data, colors, count * sizeof(ngx2::color_t));
    }
}

UNITTEST_SUITE_BEGIN(sbr)
{
    UNITTEST_FIXTURE(slice_based_renderer)
    {
        UNITTEST_FIXTURE_SETUP() {}
        UNITTEST_FIXTURE_TEARDOWN() {}

        UNITTEST_TEST(native_rgb565_a1_clips_to_slice)
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

            ngx2::color_t pixels[2 * 2] = {};
            ngx2::nsbr::ctx_t context = {pixels, 2, 2, 10, 0, 0, nullptr};
            ngx2::nsbr::draw_sprite(context, &sprite, -1, 10);

            CHECK(pixels[0] == 0x0000);
            CHECK(pixels[1] == 0xffff);
            CHECK(pixels[2] == 0xffff);
            CHECK(pixels[3] == 0x0000);
        }

        UNITTEST_TEST(native_indexed_sprite_uses_context_palette)
        {
            const u8            indices[] = {1, 0};
            const ngx2::color_t colors[]  = {0xf800, 0x07e0};
            ngx2::sprite_t      sprite    = {};
            sprite.width                  = 2;
            sprite.height                 = 1;
            sprite.pixel_format           = ngx2::FMT_PIXEL_I8;
            sprite.alpha_format           = ngx2::FMT_ALPHA_A0;
            init_bytes(sprite.pixel_data, indices, sizeof(indices));

            ngx2::palette_t palette = {};
            init_palette(palette, colors, 2);
            ngx2::color_t pixels[2] = {};
            ngx2::nsbr::ctx_t context = {pixels, 2, 1, 4, 0, 0, &palette};
            ngx2::nsbr::draw_sprite(context, &sprite, 0, 4);

            CHECK(pixels[0] == 0x07e0);
            CHECK(pixels[1] == 0xf800);
        }

        UNITTEST_TEST(scaled_sprite_uses_fixed_point_source_coordinates)
        {
            const ngx2::color_t source[] = {0xf800, 0x07e0, 0x001f, 0xffff};
            ngx2::sprite_t      sprite   = {};
            sprite.width                 = 2;
            sprite.height                = 2;
            sprite.pixel_format          = ngx2::FMT_PIXEL_RGB565;
            sprite.alpha_format          = ngx2::FMT_ALPHA_A0;
            init_bytes(sprite.pixel_data, source, sizeof(source));

            ngx2::color_t pixels[3 * 2] = {};
            ngx2::nsbr::ctx_t context = {pixels, 3, 2, 5, 0, 0, nullptr};
            ngx2::nsbr::draw_sprite(context, &sprite, -1, 5, 4, 2);

            CHECK(pixels[0] == 0xf800);
            CHECK(pixels[1] == 0x07e0);
            CHECK(pixels[2] == 0x07e0);
            CHECK(pixels[3] == 0x001f);
            CHECK(pixels[4] == 0xffff);
            CHECK(pixels[5] == 0xffff);
        }

        UNITTEST_TEST(alpha_only_sprite_uses_current_color)
        {
            const u8 alpha[] = {0x8f};
            const ngx2::color_t colors[] = {0xffff};
            ngx2::sprite_t sprite = {};
            sprite.width = 2;
            sprite.height = 1;
            sprite.alpha_format = ngx2::FMT_ALPHA_A4;
            init_bytes(sprite.alpha_data, alpha, sizeof(alpha));

            ngx2::palette_t palette = {};
            init_palette(palette, colors, 1);
            ngx2::color_t pixels[2] = {};
            ngx2::nsbr::ctx_t context = {pixels, 2, 1, 0, 0, 0, &palette};
            ngx2::nsbr::draw_sprite(context, &sprite, 0, 0);

            CHECK(pixels[0] == 0x7bef);
            CHECK(pixels[1] == 0xffff);
        }
    }
}
UNITTEST_SUITE_END
