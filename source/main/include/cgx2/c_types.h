#ifndef __CGX2_TYPES_H__
#define __CGX2_TYPES_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
    #pragma once
#endif

namespace ncore
{
    namespace ngx2
    {
        typedef u16 color_t;

        struct rect_t
        {
            i16 x;
            i16 y;
            i16 w;
            i16 h;
        };

        enum image_format_t
        {
            FMT_PIXEL_RGB565 = 0x12,  // RGB565 (16-bit) with no alpha
            FMT_PIXEL_I8     = 0x31,  // Indexed 8-bit (with RGBA palette)
        };

        enum alpha_format_t
        {
            FMT_ALPHA_A0 = 0,  // No alpha (fully opaque)
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

            i32 m_offset;         // offset from the start of the string_t to the first character
            u16 m_size_in_bytes;  // number of characters in the string (not including null terminator)
            u16 m_size_in_runes;  // number of Unicode code points (runes) in the string
        };

        template <typename K, typename V>
        struct map_t
        {
            inline u32 size() const { return m_num_entries; }
            inline K*  keys() { return (K*)((u8*)this + m_key_offset); }
            inline V*  values() { return (V*)((u8*)this + m_value_offset); }
            inline K*  key(u32 index) { return (index < m_num_entries) ? &(keys()[index]) : nullptr; }
            inline V*  value(u32 index) { return (index < m_num_entries) ? &(values()[index]) : nullptr; }

            i32 m_key_offset;    // offset from the start of the map_t to the first key
            i32 m_value_offset;  // offset from the start of the map_t to the first value
            u32 m_num_entries;   // number of key-value pairs in the map
        };

        template <typename T>
        struct array_t
        {
            inline u32      size() const { return m_size; }
            inline T*       data() { return (T*)((u8*)this + m_offset); }
            inline T const* data() const { return (T const*)((u8 const*)this + m_offset); }
            inline T*       item(u32 index) { return (index < m_size) ? &(data()[index]) : nullptr; }
            inline T const* item(u32 index) const { return (index < m_size) ? &(data()[index]) : nullptr; }

            i32 m_offset;
            u32 m_size;
        };

        struct image_descr_t
        {
            u16            m_width;     // width in pixels
            u16            m_height;    // height in pixels
            u16            m_reserved;  // reserved for future use (0)
            image_format_t m_format;    // pixel format
        };

        struct image_t
        {
            image_descr_t m_descr;   // image description (width, height, format)
            void*         m_pixels;  // pixel data
        };

        // Framebuffer is RGB565 format, 16-bit per pixel, no alpha channel
        struct framebuffer_t
        {
            u16      m_width;   // width in pixels
            u16      m_height;  // height in pixels
            color_t* m_pixels;  // pixel data
        };

        struct sprite_t
        {
            u16           m_width;          //
            u16           m_height;         //
            u8            m_pixel_format;   // image_format_t
            u8            m_alpha_format;   // alpha_format_t; packed samples are stored most-significant bits first
            u8            m_palette_index;  //
            u8            m_reserved;       //
            array_t<byte> m_pixel_data;     //
            array_t<byte> m_alpha_data;     // packed alpha map; each row starts on a byte boundary
        };

        struct sprite_pack_t
        {
            u32               m_version;  // version of the sprite pack format
            array_t<sprite_t> sprites;    // <sprite_t>, array of sprites
        };

        struct palette_t
        {
            u32           m_format;  // image_format_t
            array_t<byte> m_data;    // array of RGB565 colors
        };

        struct palette_pack_t
        {
            u32                m_version;   // version of the palette pack format
            array_t<palette_t> m_palettes;  // <palette_t>, array of palettes
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

        // Note: ascent/descent/line_gap are all in 8.8 fixed-point format and relative to size == 32
        struct font_t
        {
            array_t<u8>                 m_data;               // actual font data (bitmap or SDF)
            array_t<i8>                 m_glyphs_advance_x;   // horizontal distance from the pen position to the next character's pen position, indexed as glyph[map[ASCII character]]
            array_t<glyph_bearing_t>    m_glyphs_bearing;     // glyphs array, indexed as glyph[map[ASCII character]]
            array_t<glyph_dimensions_t> m_glyphs_dimensions;  // glyphs array, indexed as glyph[map[ASCII character]]
            array_t<u16>                m_offsets;            // offset = (offsets[map[ASCII character]] * 8) into the coverage data for the glyph
            array_t<u8>                 m_map;                // maps ASCII character codes to glyph indices in the glyphs array, or 0xFF if the character is not supported
            u8                          m_ascent;             // distance (pixels) from baseline to top of font
            u8                          m_descent;            // distance (pixels) from baseline to bottom of font
            i8                          m_line_gap;           // distance (pixels) from bottom of one line to top of next line (can be negative)
            u8                          m_size;               // font size
        };

        struct font_pack_t
        {
            u32             m_version;  // version of the font pack format
            array_t<font_t> fonts;      // array of fonts
        };

    }  // namespace ngx2
}  // namespace ncore

#endif  /// __CGX2_TYPES_H__
