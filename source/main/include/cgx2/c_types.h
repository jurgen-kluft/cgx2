#ifndef __CGX2_TYPES_H__
#define __CGX2_TYPES_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
    #pragma once
#endif

namespace ncore
{
    class alloc_t;

    namespace ngx2
    {
        // ============================================================================
        // Forward-Declarations (Opaque Types)
        // ============================================================================

        struct framebuffer_t;
        struct draw_state_t;
        struct sprite_context_t;
        struct font_context_t;
        struct sprite_t;
        struct font_t;

        // ============================================================================
        // Geometry & Utility Structs
        // ============================================================================

        struct rect_t
        {
            i32 x, y, w, h;
        };

        struct color_t
        {
            u8 r, g, b, a;
        };

        inline u16 color_to_rgb565(const color_t& c)
        {
            const u16 r5 = c.r >> 3;
            const u16 g6 = c.g >> 2;
            const u16 b5 = c.b >> 3;
            return (r5 << 11) | (g6 << 5) | b5;
        }

        enum image_format_t
        {
            FORMAT_RGB565   = 0x0102,
            FORMAT_RGB565A1 = 0x0112,
            FORMAT_RGB565A4 = 0x0122,
            FORMAT_RGB565A8 = 0x0132,
            FORMAT_RGBA8888 = 0x0204,
            FORMAT_I8       = 0x0301,
        };

        inline u32 bytes_per_pixel(image_format_t format) { return format & 0xF; }
        inline u32 bytes_per_row(image_format_t format, u32 width) { return width * bytes_per_pixel(format); }

        // ============================================================================
        // Framebuffer
        // ============================================================================

        struct framedescr_t
        {
            u16            width;     // width in pixels
            u16            height;    // height in pixels
            u16            reserved;  // reserved for future use (0)
            image_format_t format;    // pixel format
        };

        inline framedescr_t init_framedescr(u16 width, u16 height, image_format_t format)
        {
            framedescr_t descr;
            descr.width    = width;
            descr.height   = height;
            descr.reserved = 0;
            descr.format   = format;
            return descr;
        }

        inline u32 bytes_per_frame(framedescr_t const& descr) { return descr.height * descr.width * bytes_per_pixel(descr.format); }

    }  // namespace ngx2
}  // namespace ncore

#endif  /// __CGX2_TYPES_H__
