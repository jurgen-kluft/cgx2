#include "ccore/c_target.h"
#include "ccore/c_math.h"
#include "ccore/c_memory.h"

#include "cgx2/c_renderer.h"
#include "cgx2/c_color.h"
#include "cgx2/c_sprite.h"
#include "cgx2/c_palette.h"
#include "cgx2/c_font.h"
#include "cgx2/c_types.h"

#include <cmath>

namespace ncore
{
    namespace ngx2
    {
        static inline bool s_is_valid_context(ctx_t const& ctx) { return ctx.m_pixels != nullptr && ctx.m_width != 0 && ctx.m_height != 0; }

        static inline bool s_is_y_in_slice(ctx_t const& ctx, i32 y) { return y >= (i32)ctx.m_offseth && y < (i32)ctx.m_offseth + (i32)ctx.m_height; }

        static inline u16* s_row(ctx_t& ctx, i32 y) { return ctx.m_pixels + (y - (i32)ctx.m_offseth) * (i32)ctx.m_width; }

        static bool s_clip_hspan(ctx_t const& ctx, i32& x0, i32& x1, i32 y)
        {
            if (!s_is_valid_context(ctx) || !s_is_y_in_slice(ctx, y))
                return false;

            if (x0 > x1)
            {
                i32 const x = x0;
                x0          = x1;
                x1          = x;
            }

            if (x1 < 0 || x0 >= (i32)ctx.m_width)
                return false;

            if (x0 < 0)
                x0 = 0;
            if (x1 >= (i32)ctx.m_width)
                x1 = (i32)ctx.m_width - 1;
            return true;
        }

        static bool s_clip_vspan(ctx_t const& ctx, i32 x, i32& y0, i32& y1)
        {
            if (!s_is_valid_context(ctx) || x < 0 || x >= (i32)ctx.m_width)
                return false;

            if (y0 > y1)
            {
                i32 const y = y0;
                y0          = y1;
                y1          = y;
            }

            i32 const slice_y0 = (i32)ctx.m_offseth;
            i32 const slice_y1 = slice_y0 + (i32)ctx.m_height - 1;
            if (y1 < slice_y0 || y0 > slice_y1)
                return false;

            if (y0 < slice_y0)
                y0 = slice_y0;
            if (y1 > slice_y1)
                y1 = slice_y1;
            return true;
        }

        static bool s_clip_rect(ctx_t const& ctx, i32& x0, i32& y0, i32& x1, i32& y1)
        {
            if (!s_is_valid_context(ctx))
                return false;

            i32 const slice_y0 = (i32)ctx.m_offseth;
            i32 const slice_y1 = slice_y0 + (i32)ctx.m_height;
            if (x1 <= 0 || x0 >= (i32)ctx.m_width || y1 <= slice_y0 || y0 >= slice_y1)
                return false;

            if (x0 < 0)
                x0 = 0;
            if (y0 < slice_y0)
                y0 = slice_y0;
            if (x1 > (i32)ctx.m_width)
                x1 = (i32)ctx.m_width;
            if (y1 > slice_y1)
                y1 = slice_y1;
            return x0 < x1 && y0 < y1;
        }

        static inline u16 s_blend_rgb565_a2(u16 background, u16 foreground, u8 coverage)
        {
            if (coverage == 0)
                return background;
            if (coverage == 3)
                return foreground;

            u32 background_channel = (background >> 11) & 0x1F;
            u32 foreground_channel = (foreground >> 11) & 0x1F;
            u32 output_red         = background_channel + (((i32)(foreground_channel - background_channel) * coverage) >> 2);
            background_channel     = (background >> 5) & 0x3F;
            foreground_channel     = (foreground >> 5) & 0x3F;
            u32 output_green       = background_channel + (((i32)(foreground_channel - background_channel) * coverage) >> 2);
            background_channel     = background & 0x1F;
            foreground_channel     = foreground & 0x1F;
            u32 output_blue        = background_channel + (((i32)(foreground_channel - background_channel) * coverage) >> 2);
            return (u16)((output_red << 11) | (output_green << 5) | output_blue);
        }

        static inline u16 s_blend_rgb565_a4(u16 background, u16 foreground, u8 coverage)
        {
            if (coverage == 0)
                return background;
            if (coverage == 15)
                return foreground;

            u32 background_channel = (background >> 11) & 0x1F;
            u32 foreground_channel = (foreground >> 11) & 0x1F;
            u32 output_red         = background_channel + (((i32)(foreground_channel - background_channel) * coverage) >> 4);
            background_channel     = (background >> 5) & 0x3F;
            foreground_channel     = (foreground >> 5) & 0x3F;
            u32 output_green       = background_channel + (((i32)(foreground_channel - background_channel) * coverage) >> 4);
            background_channel     = background & 0x1F;
            foreground_channel     = foreground & 0x1F;
            u32 output_blue        = background_channel + (((i32)(foreground_channel - background_channel) * coverage) >> 4);
            return (u16)((output_red << 11) | (output_green << 5) | output_blue);
        }

        static inline u16 s_blend_rgb565_a8(u16 background, u16 foreground, u8 coverage)
        {
            u32 const alpha   = coverage + (coverage >> 7);
            u32 const inverse = 256 - alpha;
            u32 const red     = (((foreground & 0xF800) * alpha + (background & 0xF800) * inverse) >> 8) & 0xF800;
            u32 const green   = (((foreground & 0x07E0) * alpha + (background & 0x07E0) * inverse) >> 8) & 0x07E0;
            u32 const blue    = ((foreground & 0x001F) * alpha + (background & 0x001F) * inverse) >> 8;
            return (u16)(red | green | blue);
        }

        static void s_vspan(ctx_t& ctx, i32 x, i32 y0, i32 y1, u16 color)
        {
            u16* pixels = s_row(ctx, y0);
            for (i32 y = y0; y <= y1; ++y)
            {
                pixels[x] = color;
                pixels += ctx.m_width;
            }
        }

        void draw_vline(ctx_t& ctx, i32 x, i32 y0, i32 y1, u32 color)
        {
            if (!s_clip_vspan(ctx, x, y0, y1))
                return;
            const u16 c = rgb888_to_rgb565(color);
            s_vspan(ctx, x, y0, y1, c);
        }

        static void s_hspan(ctx_t& ctx, i32 x0, i32 x1, i32 y, u16 color)
        {
            u16* pixels = s_row(ctx, y);
            for (i32 x = x0; x <= x1; ++x)
                pixels[x] = color;
        }

        void draw_hline(ctx_t& ctx, i32 x0, i32 x1, i32 y, u32 color)
        {
            if (!s_clip_hspan(ctx, x0, x1, y))
                return;
            const u16 c = rgb888_to_rgb565(color);
            s_hspan(ctx, x0, x1, y, c);
        }

        void draw_hdline(ctx_t& ctx, i32 x0, i32 x1, i32 y, u16 dash1, u16 dash2, u32 color)
        {
            if (x0 > x1)
            {
                i32 const x = x0;
                x0          = x1;
                x1          = x;
            }
            i32 const start_x = x0;
            if (!s_clip_hspan(ctx, x0, x1, y))
                return;

            u32 period = (u32)dash1 + (u32)dash2;
            if (period == 0)
                period = 1;

            const u16 c = rgb888_to_rgb565(color);

            u16* pixels = s_row(ctx, y);
            for (i32 x = x0; x <= x1; ++x)
            {
                if ((u32)(x - start_x) % period < dash1)
                    pixels[x] = c;
            }
        }

        void draw_vdline(ctx_t& ctx, i32 x, i32 y0, i32 y1, u16 dash1, u16 dash2, u32 color)
        {
            if (y0 > y1)
            {
                i32 const y = y0;
                y0          = y1;
                y1          = y;
            }
            i32 const start_y = y0;
            if (!s_clip_vspan(ctx, x, y0, y1))
                return;

            u32 period = (u32)dash1 + (u32)dash2;
            if (period == 0)
                period = 1;

            const u16 c = rgb888_to_rgb565(color);

            for (i32 y = y0; y <= y1; ++y)
            {
                if ((u32)(y - start_y) % period < dash1)
                    s_row(ctx, y)[x] = c;
            }
        }

        static u16* s_get_rgb565_pixels(sprite_t const* sprite)
        {
            if (sprite->m_pixel_format != FMT_PIXEL_RGB565 || sprite->m_pixel_data.m_size == 0 || sprite->m_pixel_data.data() == nullptr)
                return nullptr;
            return (u16*)sprite->m_pixel_data.data();
        }

        static u8* s_get_i8_pixels(sprite_t const* sprite)
        {
            if (sprite->m_pixel_format != FMT_PIXEL_I8 || sprite->m_pixel_data.m_size == 0 || sprite->m_pixel_data.data() == nullptr)
                return nullptr;
            return (u8*)sprite->m_pixel_data.data();
        }

        static u8* s_get_alpha_data(sprite_t const* sprite, u32& alpha_row_stride)
        {
            static u8 c_opaque_alpha = 0;
            if (sprite->m_alpha_format == FMT_ALPHA_A0)
            {
                alpha_row_stride = sprite->m_width;
                return &c_opaque_alpha;
            }
            if (sprite->m_alpha_data.m_size == 0 || sprite->m_alpha_data.data() == nullptr)
                return nullptr;
            switch (sprite->m_alpha_format)
            {
                case FMT_ALPHA_A1: alpha_row_stride = ((u32)sprite->m_width + 7) >> 3; break;
                case FMT_ALPHA_A2: alpha_row_stride = ((u32)sprite->m_width + 3) >> 2; break;
                case FMT_ALPHA_A4: alpha_row_stride = ((u32)sprite->m_width + 1) >> 1; break;
                default: alpha_row_stride = sprite->m_width; break;
            }
            return (u8*)sprite->m_alpha_data.data();
        }

        static u16* s_get_palette(sprite_t const* sprite, palette_pack_t const* palette_pack)
        {
            const palette_t* palette = get_palette(palette_pack, sprite->m_palette_index);
            if (palette == nullptr || palette->m_data.m_size == 0 || palette->m_data.data() == nullptr)
                return nullptr;
            return (u16*)palette->m_data.data();
        }

        void draw_sprite(ctx_t& ctx, const sprite_t* sprite, const palette_t* palette, i32 x, i32 y, u32 color)
        {
            i32 draw_x0 = x;
            i32 draw_y0 = y;
            i32 draw_x1 = x + (i32)sprite->m_width;
            i32 draw_y1 = y + (i32)sprite->m_height;
            if (!s_clip_rect(ctx, draw_x0, draw_y0, draw_x1, draw_y1))
                return;

            i32 const source_x0   = draw_x0 - x;
            i32 const source_y0   = draw_y0 - y;
            i32 const span_width  = draw_x1 - draw_x0;
            i32 const span_height = draw_y1 - draw_y0;

            const u16* rgb565_pixels = s_get_rgb565_pixels(sprite);
            u8 const*  i8_pixels     = s_get_i8_pixels(sprite);
            if (rgb565_pixels != nullptr)
            {
                u32       alpha_row_stride = 0;
                const u8* alpha_pixels     = s_get_alpha_data(sprite, alpha_row_stride);
                if (alpha_pixels != nullptr)
                {
                    switch (sprite->m_alpha_format)
                    {
                        case FMT_ALPHA_A0:
                            for (i32 row = 0; row < span_height; ++row)
                            {
                                u16 const* source      = rgb565_pixels + (source_y0 + row) * sprite->m_width + source_x0;
                                u16*       destination = s_row(ctx, draw_y0 + row) + draw_x0;
                                for (i32 column = 0; column < span_width; ++column)
                                    *destination++ = *source++;
                            }
                            break;
                        case FMT_ALPHA_A1:
                            for (i32 row = 0; row < span_height; ++row)
                            {
                                u16 const* source      = rgb565_pixels + (source_y0 + row) * sprite->m_width + source_x0;
                                u8 const*  alpha       = alpha_pixels + (source_y0 + row) * alpha_row_stride + (source_x0 >> 3);
                                i32        shift       = 7 - (source_x0 & 7);
                                u16*       destination = s_row(ctx, draw_y0 + row) + draw_x0;
                                for (i32 column = 0; column < span_width; ++column, ++source, ++destination)
                                {
                                    if (((*alpha >> shift) & 1) != 0)
                                        *destination = *source;
                                    if (shift == 0)
                                    {
                                        ++alpha;
                                        shift = 7;
                                    }
                                    else
                                    {
                                        --shift;
                                    }
                                }
                            }
                            break;
                        case FMT_ALPHA_A2:
                            for (i32 row = 0; row < span_height; ++row)
                            {
                                u16 const* source      = rgb565_pixels + (source_y0 + row) * sprite->m_width + source_x0;
                                u8 const*  alpha       = alpha_pixels + (source_y0 + row) * alpha_row_stride + (source_x0 >> 2);
                                i32        shift       = 6 - ((source_x0 & 3) << 1);
                                u16*       destination = s_row(ctx, draw_y0 + row) + draw_x0;
                                for (i32 column = 0; column < span_width; ++column, ++source, ++destination)
                                {
                                    *destination = s_blend_rgb565_a2(*destination, *source, (u8)((*alpha >> shift) & 3));
                                    if (shift == 0)
                                    {
                                        ++alpha;
                                        shift = 6;
                                    }
                                    else
                                    {
                                        shift -= 2;
                                    }
                                }
                            }
                            break;
                        case FMT_ALPHA_A4:
                            for (i32 row = 0; row < span_height; ++row)
                            {
                                u16 const* source      = rgb565_pixels + (source_y0 + row) * sprite->m_width + source_x0;
                                u8 const*  alpha       = alpha_pixels + (source_y0 + row) * alpha_row_stride + (source_x0 >> 1);
                                bool       high        = (source_x0 & 1) == 0;
                                u16*       destination = s_row(ctx, draw_y0 + row) + draw_x0;
                                for (i32 column = 0; column < span_width; ++column, ++source, ++destination)
                                {
                                    *destination = s_blend_rgb565_a4(*destination, *source, high ? (u8)(*alpha >> 4) : (u8)(*alpha & 0x0F));
                                    if (!high)
                                        ++alpha;
                                    high = !high;
                                }
                            }
                            break;
                        case FMT_ALPHA_A8:
                            for (i32 row = 0; row < span_height; ++row)
                            {
                                u16 const* source      = rgb565_pixels + (source_y0 + row) * sprite->m_width + source_x0;
                                u8 const*  alpha       = alpha_pixels + (source_y0 + row) * alpha_row_stride + source_x0;
                                u16*       destination = s_row(ctx, draw_y0 + row) + draw_x0;
                                for (i32 column = 0; column < span_width; ++column, ++source, ++alpha, ++destination)
                                    *destination = s_blend_rgb565_a8(*destination, *source, *alpha);
                            }
                            break;
                        default: break;
                    }
                }
            }
            else if (i8_pixels != nullptr)
            {
                u32        alpha_row_stride = 0;
                const u8*  alpha_pixels     = s_get_alpha_data(sprite, alpha_row_stride);
                if (alpha_pixels != nullptr && palette != nullptr)
                {
                    u16 const * pal = (u16 const*)palette->m_data.data();
                    switch (sprite->m_alpha_format)
                    {
                        case FMT_ALPHA_A0:
                            for (i32 row = 0; row < span_height; ++row)
                            {
                                u8 const* source      = i8_pixels + (source_y0 + row) * sprite->m_width + source_x0;
                                u16*      destination = s_row(ctx, draw_y0 + row) + draw_x0;
                                for (i32 column = 0; column < span_width; ++column)
                                    *destination++ = pal[*source++];
                            }
                            break;
                        case FMT_ALPHA_A1:
                            for (i32 row = 0; row < span_height; ++row)
                            {
                                u8 const* source      = i8_pixels + (source_y0 + row) * sprite->m_width + source_x0;
                                u8 const* alpha       = alpha_pixels + (source_y0 + row) * alpha_row_stride + (source_x0 >> 3);
                                i32       shift       = 7 - (source_x0 & 7);
                                u16*      destination = s_row(ctx, draw_y0 + row) + draw_x0;
                                for (i32 column = 0; column < span_width; ++column, ++source, ++destination)
                                {
                                    if (((*alpha >> shift) & 1) != 0)
                                        *destination = pal[*source];
                                    if (shift == 0)
                                    {
                                        ++alpha;
                                        shift = 7;
                                    }
                                    else
                                    {
                                        --shift;
                                    }
                                }
                            }
                            break;
                        case FMT_ALPHA_A2:
                            for (i32 row = 0; row < span_height; ++row)
                            {
                                u8 const* source      = i8_pixels + (source_y0 + row) * sprite->m_width + source_x0;
                                u8 const* alpha       = alpha_pixels + (source_y0 + row) * alpha_row_stride + (source_x0 >> 2);
                                i32       shift       = 6 - ((source_x0 & 3) << 1);
                                u16*      destination = s_row(ctx, draw_y0 + row) + draw_x0;
                                for (i32 column = 0; column < span_width; ++column, ++source, ++destination)
                                {
                                    *destination = s_blend_rgb565_a2(*destination, pal[*source], (u8)((*alpha >> shift) & 3));
                                    if (shift == 0)
                                    {
                                        ++alpha;
                                        shift = 6;
                                    }
                                    else
                                    {
                                        shift -= 2;
                                    }
                                }
                            }
                            break;
                        case FMT_ALPHA_A4:
                            for (i32 row = 0; row < span_height; ++row)
                            {
                                u8 const* source      = i8_pixels + (source_y0 + row) * sprite->m_width + source_x0;
                                u8 const* alpha       = alpha_pixels + (source_y0 + row) * alpha_row_stride + (source_x0 >> 1);
                                bool      high        = (source_x0 & 1) == 0;
                                u16*      destination = s_row(ctx, draw_y0 + row) + draw_x0;
                                for (i32 column = 0; column < span_width; ++column, ++source, ++destination)
                                {
                                    *destination = s_blend_rgb565_a4(*destination, pal[*source], high ? (u8)(*alpha >> 4) : (u8)(*alpha & 0x0F));
                                    if (!high)
                                        ++alpha;
                                    high = !high;
                                }
                            }
                            break;
                        case FMT_ALPHA_A8:
                            for (i32 row = 0; row < span_height; ++row)
                            {
                                u8 const* source      = i8_pixels + (source_y0 + row) * sprite->m_width + source_x0;
                                u8 const* alpha       = alpha_pixels + (source_y0 + row) * alpha_row_stride + source_x0;
                                u16*      destination = s_row(ctx, draw_y0 + row) + draw_x0;
                                for (i32 column = 0; column < span_width; ++column, ++source, ++alpha, ++destination)
                                    *destination = s_blend_rgb565_a8(*destination, pal[*source], *alpha);
                            }
                            break;
                        default: break;
                    }
                }
            }
            else
            {
                u32       alpha_row_stride = 0;
                const u8* alpha_pixels     = s_get_alpha_data(sprite, alpha_row_stride);
                if (alpha_pixels != nullptr)
                {
                    const u16 solid_color = rgb888_to_rgb565(color);
                    switch (sprite->m_alpha_format)
                    {
                        case FMT_ALPHA_A1:
                            for (i32 row = 0; row < span_height; ++row)
                            {
                                u8 const* alpha       = alpha_pixels + (source_y0 + row) * alpha_row_stride + (source_x0 >> 3);
                                i32       shift       = 7 - (source_x0 & 7);
                                u16*      destination = s_row(ctx, draw_y0 + row) + draw_x0;
                                for (i32 column = 0; column < span_width; ++column, ++destination)
                                {
                                    if (((*alpha >> shift) & 1) != 0)
                                        *destination = solid_color;
                                    if (shift == 0)
                                    {
                                        ++alpha;
                                        shift = 7;
                                    }
                                    else
                                    {
                                        --shift;
                                    }
                                }
                            }
                            break;
                        case FMT_ALPHA_A2:
                            for (i32 row = 0; row < span_height; ++row)
                            {
                                u8 const* alpha       = alpha_pixels + (source_y0 + row) * alpha_row_stride + (source_x0 >> 2);
                                i32       shift       = 6 - ((source_x0 & 3) << 1);
                                u16*      destination = s_row(ctx, draw_y0 + row) + draw_x0;
                                for (i32 column = 0; column < span_width; ++column, ++destination)
                                {
                                    *destination = s_blend_rgb565_a2(*destination, solid_color, (u8)((*alpha >> shift) & 3));
                                    if (shift == 0)
                                    {
                                        ++alpha;
                                        shift = 6;
                                    }
                                    else
                                    {
                                        shift -= 2;
                                    }
                                }
                            }
                            break;
                        case FMT_ALPHA_A4:
                            for (i32 row = 0; row < span_height; ++row)
                            {
                                u8 const* alpha       = alpha_pixels + (source_y0 + row) * alpha_row_stride + (source_x0 >> 1);
                                bool      high        = (source_x0 & 1) == 0;
                                u16*      destination = s_row(ctx, draw_y0 + row) + draw_x0;
                                for (i32 column = 0; column < span_width; ++column, ++destination)
                                {
                                    *destination = s_blend_rgb565_a4(*destination, solid_color, high ? (u8)(*alpha >> 4) : (u8)(*alpha & 0x0F));
                                    if (!high)
                                        ++alpha;
                                    high = !high;
                                }
                            }
                            break;
                        case FMT_ALPHA_A8:
                            for (i32 row = 0; row < span_height; ++row)
                            {
                                u8 const* alpha       = alpha_pixels + (source_y0 + row) * alpha_row_stride + source_x0;
                                u16*      destination = s_row(ctx, draw_y0 + row) + draw_x0;
                                for (i32 column = 0; column < span_width; ++column, ++alpha, ++destination)
                                    *destination = s_blend_rgb565_a8(*destination, solid_color, *alpha);
                            }
                            break;
                        default: break;
                    }
                }
            }
        }

        void draw_sprite(ctx_t& ctx, const sprite_t* sprite, const palette_t* palette, i32 x, i32 y, i32 width, i32 height, u32 color)
        {
            i32 const destination_x1 = x + width;
            i32 const destination_y1 = y + height;

            i32 draw_x0 = x;
            i32 draw_y0 = y;
            i32 draw_x1 = (i32)destination_x1;
            i32 draw_y1 = (i32)destination_y1;
            if (!s_clip_rect(ctx, draw_x0, draw_y0, draw_x1, draw_y1))
                return;

            i32 const source_step_x = ((i32)sprite->m_width << 16) / width;
            i32 const source_step_y = ((i32)sprite->m_height << 16) / height;

#define SBR_SCALED_LOOP(pixel_operation)                                                                                                \
    {                                                                                                                                   \
        i32 source_y_fixed = (i32)(draw_y0 - y) * source_step_y;                                                                        \
        for (i32 destination_y = draw_y0; destination_y < draw_y1; ++destination_y, source_y_fixed += source_step_y)                    \
        {                                                                                                                               \
            i32 const source_y       = (i32)(source_y_fixed >> 16);                                                                     \
            u16*      destination    = s_row(ctx, destination_y) + draw_x0;                                                             \
            i32       source_x_fixed = (i32)(draw_x0 - x) * source_step_x;                                                              \
            for (i32 destination_x = draw_x0; destination_x < draw_x1; ++destination_x, ++destination, source_x_fixed += source_step_x) \
            {                                                                                                                           \
                i32 const source_x = (i32)(source_x_fixed >> 16);                                                                       \
                pixel_operation                                                                                                         \
            }                                                                                                                           \
        }                                                                                                                               \
    }

            const u16* rgb565_pixels = s_get_rgb565_pixels(sprite);
            if (rgb565_pixels != nullptr)
            {
                u32       alpha_row_stride = 0;
                const u8* alpha_pixels     = s_get_alpha_data(sprite, alpha_row_stride);
                if (alpha_pixels != nullptr)
                {
                    switch (sprite->m_alpha_format)
                    {
                        case FMT_ALPHA_A0: SBR_SCALED_LOOP(*destination = rgb565_pixels[source_y * sprite->m_width + source_x];) break;
                        case FMT_ALPHA_A1:
                            SBR_SCALED_LOOP(u8 const alpha = alpha_pixels[source_y * alpha_row_stride + (source_x >> 3)]; if (((alpha >> (7 - (source_x & 7))) & 1) != 0)* destination = rgb565_pixels[source_y * sprite->m_width + source_x];)
                            break;
                        case FMT_ALPHA_A2:
                            SBR_SCALED_LOOP(u8 const alpha = alpha_pixels[source_y * alpha_row_stride + (source_x >> 2)];
                                            *destination   = s_blend_rgb565_a2(*destination, rgb565_pixels[source_y * sprite->m_width + source_x], (u8)((alpha >> (6 - ((source_x & 3) << 1))) & 3));)
                            break;
                        case FMT_ALPHA_A4:
                            SBR_SCALED_LOOP(u8 const alpha = alpha_pixels[source_y * alpha_row_stride + (source_x >> 1)];
                                            *destination   = s_blend_rgb565_a4(*destination, rgb565_pixels[source_y * sprite->m_width + source_x], (source_x & 1) == 0 ? (u8)(alpha >> 4) : (u8)(alpha & 0x0F));)
                            break;
                        case FMT_ALPHA_A8: SBR_SCALED_LOOP(*destination = s_blend_rgb565_a8(*destination, rgb565_pixels[source_y * sprite->m_width + source_x], alpha_pixels[source_y * alpha_row_stride + source_x]);) break;
                        default: break;
                    }
                }
            }
            else
            {
                u32        alpha_row_stride = 0;
                const u8*  alpha_pixels     = s_get_alpha_data(sprite, alpha_row_stride);
                const u8*  i8_pixels        = s_get_i8_pixels(sprite);
                if (i8_pixels != nullptr && alpha_pixels != nullptr && palette != nullptr)
                {
                    const u16* pal = (u16 const*)palette->m_data.data();

                    switch (sprite->m_alpha_format)
                    {
                        case FMT_ALPHA_A0: SBR_SCALED_LOOP(*destination = pal[i8_pixels[source_y * sprite->m_width + source_x]];) break;
                        case FMT_ALPHA_A1:
                            SBR_SCALED_LOOP(u8 const alpha = alpha_pixels[source_y * alpha_row_stride + (source_x >> 3)]; if (((alpha >> (7 - (source_x & 7))) & 1) != 0)* destination = pal[i8_pixels[source_y * sprite->m_width + source_x]];)
                            break;
                        case FMT_ALPHA_A2:
                            SBR_SCALED_LOOP(u8 const alpha = alpha_pixels[source_y * alpha_row_stride + (source_x >> 2)];
                                            *destination   = s_blend_rgb565_a2(*destination, pal[i8_pixels[source_y * sprite->m_width + source_x]], (u8)((alpha >> (6 - ((source_x & 3) << 1))) & 3));)
                            break;
                        case FMT_ALPHA_A4:
                            SBR_SCALED_LOOP(u8 const alpha = alpha_pixels[source_y * alpha_row_stride + (source_x >> 1)];
                                            *destination   = s_blend_rgb565_a4(*destination, pal[i8_pixels[source_y * sprite->m_width + source_x]], (source_x & 1) == 0 ? (u8)(alpha >> 4) : (u8)(alpha & 0x0F));)
                            break;
                        case FMT_ALPHA_A8: SBR_SCALED_LOOP(*destination = s_blend_rgb565_a8(*destination, pal[i8_pixels[source_y * sprite->m_width + source_x]], alpha_pixels[source_y * alpha_row_stride + source_x]);) break;
                        default: break;
                    }
                }
                else
                {
                    u32       alpha_row_stride = 0;
                    const u8* alpha_pixels     = s_get_alpha_data(sprite, alpha_row_stride);
                    if (alpha_pixels != nullptr)
                    {
                        const u16 solid_color = rgb888_to_rgb565(color);
                        switch (sprite->m_alpha_format)
                        {
                            case FMT_ALPHA_A1: SBR_SCALED_LOOP(u8 const alpha = alpha_pixels[source_y * alpha_row_stride + (source_x >> 3)]; if (((alpha >> (7 - (source_x & 7))) & 1) != 0)* destination = solid_color;) break;
                            case FMT_ALPHA_A2:
                                SBR_SCALED_LOOP(u8 const alpha = alpha_pixels[source_y * alpha_row_stride + (source_x >> 2)]; *destination = s_blend_rgb565_a2(*destination, solid_color, (u8)((alpha >> (6 - ((source_x & 3) << 1))) & 3));)
                                break;
                            case FMT_ALPHA_A4:
                                SBR_SCALED_LOOP(u8 const alpha = alpha_pixels[source_y * alpha_row_stride + (source_x >> 1)]; *destination = s_blend_rgb565_a4(*destination, solid_color, (source_x & 1) == 0 ? (u8)(alpha >> 4) : (u8)(alpha & 0x0F));)
                                break;
                            case FMT_ALPHA_A8: SBR_SCALED_LOOP(*destination = s_blend_rgb565_a8(*destination, solid_color, alpha_pixels[source_y * alpha_row_stride + source_x]);) break;
                            default: break;
                        }
                    }
                }
            }
#undef SBR_SCALED_LOOP
        }

        static bool s_get_glyph(font_t const& font, u8 ascii_character, u8& glyph_index, glyph_bearing_t const*& bearing, glyph_dimensions_t const*& dimensions, i8& advance, u16& offset)
        {
            if (ascii_character > 127 || font.m_map.m_size <= ascii_character || font.m_map.data() == nullptr)
                return false;

            glyph_index = font.m_map.data()[ascii_character];
            if (glyph_index == 0xFF || font.m_glyphs_bearing.m_size <= glyph_index || font.m_glyphs_dimensions.m_size <= glyph_index || font.m_glyphs_advance_x.m_size <= glyph_index || font.m_offsets.m_size <= glyph_index)
                return false;

            bearing    = font.m_glyphs_bearing.item(glyph_index);
            dimensions = font.m_glyphs_dimensions.item(glyph_index);
            advance    = *font.m_glyphs_advance_x.item(glyph_index);
            offset     = *font.m_offsets.item(glyph_index);
            return bearing != nullptr && dimensions != nullptr;
        }

        static constexpr i32 c_fixed_fraction_bits = 16;
        static constexpr i32 c_fixed_one           = 1 << c_fixed_fraction_bits;

        static i32 s_scale_metric(i32 metric, i32 scale_fixed) { return (i32)(((i64)metric * scale_fixed) / c_fixed_one); }

        static void s_draw_glyph_sdf(ctx_t& ctx, u8 const* glyph_data, glyph_bearing_t const& bearing, glyph_dimensions_t const& dimensions, i32 pen_x, i32 pen_y, u16 color, i32 scale_fixed)
        {
            i32 const output_x      = pen_x + s_scale_metric(bearing.m_x, scale_fixed);
            i32 const output_y      = pen_y - s_scale_metric(bearing.m_y, scale_fixed);
            i32 const output_width  = s_scale_metric(dimensions.m_w, scale_fixed);
            i32 const output_height = s_scale_metric(dimensions.m_h, scale_fixed);

            i32 const output_x1 = output_x + output_width;
            i32 const output_y1 = output_y + output_height;

            i32 clip_x0 = output_x;
            i32 clip_y0 = output_y;
            i32 clip_x1 = (i32)output_x1;
            i32 clip_y1 = (i32)output_y1;
            if (!s_clip_rect(ctx, clip_x0, clip_y0, clip_x1, clip_y1))
                return;

            i32 const source_step_fixed = (i32)(((i64)c_fixed_one * c_fixed_one) / scale_fixed);
            i32       source_y_fixed    = (i32)((i64)(clip_y0 - output_y) * source_step_fixed);
            for (i32 destination_y = clip_y0; destination_y < clip_y1; ++destination_y, source_y_fixed += source_step_fixed)
            {
                i32 source_row = source_y_fixed >> c_fixed_fraction_bits;
                if (source_row >= dimensions.m_h)
                    source_row = dimensions.m_h - 1;
                i32  source_x_fixed = (i32)((i64)(clip_x0 - output_x) * source_step_fixed);
                u16* row            = s_row(ctx, destination_y);
                for (i32 destination_x = clip_x0; destination_x < clip_x1; ++destination_x, source_x_fixed += source_step_fixed)
                {
                    i32 source_column = source_x_fixed >> c_fixed_fraction_bits;
                    if (source_column >= dimensions.m_w)
                        source_column = dimensions.m_w - 1;
                    i32 const pixel_index   = source_row * dimensions.m_w + source_column;
                    u8 const  packed_sample = glyph_data[pixel_index >> 1];
                    u8 const  distance      = (pixel_index & 1) ? (packed_sample & 0x0F) : (packed_sample >> 4);
                    if (distance > 1)
                        row[destination_x] = s_blend_rgb565_a4(row[destination_x], color, distance - 1);
                }
            }
        }

        void draw_text(ctx_t& ctx, const font_t* font, u16 fontSize, i32 x, i32 y, const char* text, u32 color)
        {
            if (!s_is_valid_context(ctx) || font == nullptr || text == nullptr)
                return;

            i32 const glyph_height = (i32)font->m_ascent - (i32)font->m_descent;
            if (glyph_height <= 0)
                return;

            const u16 colr = rgb888_to_rgb565(color);

            u16 const clamped_font_size = fontSize < 9 ? 9 : (fontSize > 255 ? 255 : fontSize);
            i32 const scale_fixed       = ((i32)clamped_font_size << c_fixed_fraction_bits) / glyph_height;
            i32 const line_height       = s_scale_metric(glyph_height + (i32)font->m_line_gap, scale_fixed);
            i32       pen_x             = x;
            i32       pen_y             = y;
            while (*text != '\0')
            {
                u8 const ascii_character = (u8)*text++;
                if (ascii_character == '\n')
                {
                    pen_x = x;
                    pen_y += line_height;
                    continue;
                }
                if (ascii_character == '\r')
                    continue;

                u8                        glyph_index;
                glyph_bearing_t const*    bearing;
                glyph_dimensions_t const* dimensions;
                i8                        advance;
                u16                       offset;
                if (!s_get_glyph(*font, ascii_character, glyph_index, bearing, dimensions, advance, offset))
                    continue;

                const u8* glyph_data = font->m_data.data() + (u32)offset * 8;
                s_draw_glyph_sdf(ctx, glyph_data, *bearing, *dimensions, pen_x, pen_y, colr, scale_fixed);
                pen_x += s_scale_metric(advance, scale_fixed);
            }
        }

    }  // namespace ngx2
}  // namespace ncore
