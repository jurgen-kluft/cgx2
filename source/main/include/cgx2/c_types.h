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
        enum image_format_t
        {
            IMAGE_FORMAT_RGB565 = 0x0102,
            IMAGE_FORMAT_I8     = 0x0301,
        };

        enum palette_format_t
        {
            PALETTE_FORMAT_RGB565 = 0x0102,
        };

        struct image_descr_t
        {
            u16            width;     // width in pixels
            u16            height;    // height in pixels
            u16            reserved;  // reserved for future use (0)
            image_format_t format;    // pixel format
        };

        struct framebuffer_t
        {
            image_descr_t descr;   // framebuffer description (width, height, format)
            void*         pixels;  // pixel data
        };

        struct sprite_t
        {
            u16         width;      //
            u16         height;     //
            u16         format;     // image_format_t
            u16         reserved;   //
            u32         data_size;  //
            const void* data;       //
        };

        struct palette_t
        {
            u16         format;     // image_format_t
            u16         data_size;  // number of colors in the palette
            const byte* data;       // array of colors
        };

        struct glyph_t
        {
            i8 m_advance_x;  // how much to move the pen horizontally to the next character after drawing this one
            i8 m_bearing_x;  // horizontal distance from the pen position to the left edge of the glyph bitmap
            i8 m_bearing_y;  // vertical distance from the pen position to the top edge of the glyph bitmap (can be negative)
            u8 m_width;      // width of the glyph bitmap in pixels
            u8 m_height;     // height of the glyph bitmap in pixels
        };

        struct font_t
        {
            const u8*  m_sdf;       // actual coverage image containing all glyphs, 4-bit
            glyph_t*   m_glyphs;    // glyphs array, indexed as glyph[map[ASCII character]]
            const u32* m_offsets;   // offset = (offsets[map[ASCII character]]) into the coverage data for the glyph
            u8         m_map[128];  // maps ASCII character codes to glyph indices in the glyphs array, or 0xFF if the character is not supported
            i8         m_ascent;    // distance from baseline to top of font
            i8         m_descent;   // distance from baseline to bottom of font (negative value)
            i8         m_line_gap;  // distance from bottom of one line to top of next line (can be negative)
        };

        struct rect_t
        {
            i32 x, y, w, h;
        };

        inline bool is_y_in_rect(i32 y, rect_t const& r) { return (y >= r.y && y < r.y + r.h); }
        inline bool is_x_in_rect(i32 x, rect_t const& r) { return (x >= r.x && x < r.x + r.w); }
        inline bool is_point_in_rect(i32 px, i32 py, rect_t const& r) { return is_x_in_rect(px, r) && is_y_in_rect(py, r); }
        inline i32  clip_y_to_rect(i32 y, rect_t const& r) { return (y < r.y) ? r.y : ((y >= r.y + r.h) ? (r.y + r.h - 1) : y); }
        inline i32  clip_x_to_rect(i32 x, rect_t const& r) { return (x < r.x) ? r.x : ((x >= r.x + r.w) ? (r.x + r.w - 1) : x); }
        inline void sort_x(i32& x0, i32& x1)
        {
            if (x0 > x1)
            {
                i32 t = x0;
                x0    = x1;
                x1    = t;
            }
        }
        inline void sort_y(i32& y0, i32& y1)
        {
            if (y0 > y1)
            {
                i32 t = y0;
                y0    = y1;
                y1    = t;
            }
        }

        typedef u16 color_t;

        inline u32 bytes_per_pixel(image_format_t format) { return format & 0xF; }
        inline u32 bytes_per_row(image_format_t format, u32 width) { return width * bytes_per_pixel(format); }

        // ============================================================================
        // Framebuffer
        // ============================================================================

        inline image_descr_t init_image_descr(u16 width, u16 height, image_format_t format)
        {
            image_descr_t descr;
            descr.width    = width;
            descr.height   = height;
            descr.reserved = 0;
            descr.format   = format;
            return descr;
        }

        inline u32 bytes_per_image(image_descr_t const& descr) { return descr.height * descr.width * bytes_per_pixel(descr.format); }

    }  // namespace ngx2
}  // namespace ncore

#endif  /// __CGX2_TYPES_H__
