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
            FMT_PIXEL_RGB565   = 0x01,  // RGB565 (16-bit) with no alpha
            FMT_PIXEL_RGBA8888 = 0x02,  // RGBA8888 (32-bit)
            FMT_PIXEL_I8       = 0x03,  // Indexed 8-bit (with RGBA palette)
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

        struct slice_t
        {
            inline u32 size() { return m_size; }

            template <typename T>
            inline T* data()
            { return ((byte*)&m_offset + m_offset); }

            template <typename T>
            inline T* item(u32 index)
            {
                if (index >= m_size)
                    return nullptr;

                T* array = (T*)((byte*)&m_offset + m_offset);
                return &array[index];
            }

            template <typename T>
            inline T const* data() const
            { return ((byte const*)&m_offset + m_offset); }

            template <typename T>
            inline T const* item(u32 index) const
            {
                if (index >= m_size)
                    return nullptr;
                T const* array = (T const*)((byte const*)&m_offset + m_offset);
                return &array[index];
            }

        private:
            u32 m_size;
            i32 m_offset;
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
            u16     width;          //
            u16     height;         //
            u8      pixel_format;   // image_format_t
            u8      alpha_format;   // alpha_format_t; packed samples are stored most-significant bits first
            u8      reserved;       //
            u8      palette_index;  //
            slice_t pixel_data;     //
            slice_t alpha_data;     // packed alpha map; each row starts on a byte boundary
        };

        struct sprite_pack_t
        {
            u32     num_sprites;  // number of sprites in the pack
            slice_t sprites;      // <sprite_t>, array of sprites
        };

        struct palette_t
        {
            u32     format;  // image_format_t
            slice_t data;    // array of colors
        };

        struct palette_pack_t
        {
            u32     num_palettes;  // number of palettes in the pack
            slice_t palettes;      // <palette_t>, array of palettes
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
            slice_t m_data;               // <u8>, actual font data (bitmap or SDF)
            slice_t m_glyphs_advance_x;   // <i8>, horizontal distance from the pen position to the next character's pen position, indexed as glyph[map[ASCII character]]
            slice_t m_glyphs_bearing;     // <glyph_bearing_t>, glyphs array, indexed as glyph[map[ASCII character]]
            slice_t m_glyphs_dimensions;  // <glyph_dimensions_t>, glyphs array, indexed as glyph[map[ASCII character]]
            slice_t m_offsets;            // <u32>, offset = (offsets[map[ASCII character]]) into the coverage data for the glyph
            slice_t m_map;                // <u8>, maps ASCII character codes to glyph indices in the glyphs array, or 0xFF if the character is not supported
            i8      m_ascent;             // distance from baseline to top of font
            i8      m_descent;            // distance from baseline to bottom of font (negative value)
            i8      m_line_gap;           // distance from bottom of one line to top of next line (can be negative)
            u8      m_font_type;          // 0 = bitmap font, 1 = SDF font
        };

        struct font_pack_t
        {
            u32     num_fonts;  // number of fonts in the pack
            slice_t fonts;      // <font_t>, array of fonts
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
