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
            FMT_PIXEL_RGB565   = 0x12,  // RGB565 (16-bit) with no alpha
            FMT_PIXEL_RGBA8888 = 0x24,  // RGBA8888 (32-bit)
            FMT_PIXEL_I8       = 0x31,  // Indexed 8-bit (with RGBA palette)
        };

        enum alpha_format_t
        {
            FMT_ALPHA_A0 = 0,
            FMT_ALPHA_A1 = 1,
            FMT_ALPHA_A2 = 2,
            FMT_ALPHA_A4 = 4,
            FMT_ALPHA_A8 = 8,
        };

        enum palette_format_t
        {
            FMT_PALETTE_RGBA8888 = 0x01,  // RGBA8888 (32-bit)
            FMT_PALETTE_RGB565   = 0x02   // RGB565 (16-bit)
        };

        struct string_t
        {
            inline u32         size() const { return m_size_in_runes; }
            inline char const* str() const { return (char const*)((u8 const*)this + m_offset); }
            inline char const* end() const { return (char const*)((u8 const*)this + m_offset + m_size_in_bytes); }

            i64 m_offset;         // offset from the start of the string_t to the first character
            u32 m_size_in_bytes;  // number of characters in the string (not including null terminator)
            u32 m_size_in_runes;  // number of Unicode code points (runes) in the string
        };

        template <typename K, typename V>
        struct map_t
        {
            inline u64 size() const { return m_num_entries; }
            inline K*  keys() { return (K*)((u8*)this + m_key_offset); }
            inline V*  values() { return (V*)((u8*)this + m_value_offset); }
            inline K*  key(u32 index) { return (index < m_num_entries) ? keys()[index] : nullptr; }
            inline V*  value(u32 index) { return (index < m_num_entries) ? values()[index] : nullptr; }

            i64 m_key_offset;    // offset from the start of the map_t to the first key
            i64 m_value_offset;  // offset from the start of the map_t to the first value
            u64 m_num_entries;   // number of key-value pairs in the map
        };

        template <typename T>
        struct array_t
        {
            inline u64      size() const { return m_size; }
            inline T*       data() { return (T*)((u8*)this + m_offset); }
            inline T const* data() const { return (T const*)((u8 const*)this + m_offset); }
            inline T*       item(u32 index) { return (index < m_size) ? data()[index] : nullptr; }
            inline T const* item(u32 index) const { return (index < m_size) ? data()[index] : nullptr; }

            i64 m_offset;
            u64 m_size;
        };

        struct image_descr_t
        {
            u16            width;     // width in pixels
            u16            height;    // height in pixels
            u16            reserved;  // reserved for future use (0)
            image_format_t format;    // pixel format
        };

        struct image_t
        {
            image_descr_t descr;   // image description (width, height, format)
            void*         pixels;  // pixel data
        };

        // Framebuffer is RGB565 format, 16-bit per pixel, no alpha channel
        struct framebuffer_t
        {
            u16   width;   // width in pixels
            u16   height;  // height in pixels
            void* pixels;  // pixel data
        };

        struct sprite_t
        {
            u16           width;          //
            u16           height;         //
            u8            pixel_format;   // image_format_t
            u8            alpha_format;   // alpha_format_t; packed samples are stored most-significant bits first
            u8            palette_index;  //
            u8            reserved;       //
            array_t<byte> pixel_data;     //
            array_t<byte> alpha_data;     // packed alpha map; each row starts on a byte boundary
        };

        struct sprite_pack_t
        {
            array_t<sprite_t> sprites;  // <sprite_t>, array of sprites
        };

        struct palette_t
        {
            u32           format;  // image_format_t
            array_t<byte> data;    // array of colors
        };

        struct palette_pack_t
        {
            array_t<palette_t> palettes;  // <palette_t>, array of palettes
        };

        struct glyph_bearing_t
        {
            i8 m_x;  // horizontal distance from the pen position to the left edge of the glyph bitmap
            i8 m_y;  // vertical distance from the pen position to the top edge of the glyph bitmap (can be negative)
        };

        struct glyph_dimensions_t
        {
            u8 m_w;  // width of the glyph bitmap in pixels
            u8 m_h;  // height of the glyph bitmap in pixels
        };

        struct font_t
        {
            array_t<u8>                 m_data;               // actual font data (bitmap or SDF)
            array_t<i8>                 m_glyphs_advance_x;   // horizontal distance from the pen position to the next character's pen position, indexed as glyph[map[ASCII character]]
            array_t<glyph_bearing_t>    m_glyphs_bearing;     // glyphs array, indexed as glyph[map[ASCII character]]
            array_t<glyph_dimensions_t> m_glyphs_dimensions;  // glyphs array, indexed as glyph[map[ASCII character]]
            array_t<u16>                m_offsets;            // offset = (offsets[map[ASCII character]] * 8) into the coverage data for the glyph
            array_t<u8>                 m_map;                // maps ASCII character codes to glyph indices in the glyphs array, or 0xFF if the character is not supported
            i8                          m_ascent;             // distance from baseline to top of font
            i8                          m_descent;            // distance from baseline to bottom of font (negative value)
            i8                          m_line_gap;           // distance from bottom of one line to top of next line (can be negative)
            u8                          m_font_type;          // 0 = bitmap font, 1 = SDF font
        };

        struct font_pack_t
        {
            array_t<font_t> fonts;  // array of fonts
        };

        struct rect_t
        {
            i16 x, y, w, h;
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
