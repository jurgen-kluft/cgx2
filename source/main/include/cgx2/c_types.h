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
        struct context_t;
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
        // Blending
        // ============================================================================

        struct blend_state_t
        {
            u8 alpha;             // 0..255
            u8 ignore_src_alpha;  // 0 or 1
        };

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

        // ============================================================================
        // Draw State
        // ============================================================================

        struct draw_state_t
        {
            color_t       color;     // current drawing color
            blend_state_t blend;     // current blend state
            u32           sa;        // calculated source alpha (0..256) for blending, derived from color.a and blend.alpha
            rect_t        scissor;   // active scissor rect
            u8            fill;      // 0 or 1
            f32           rotation;  // degrees
            f32           scale_x;   // horizontal scale factor
            f32           scale_y;   // vertical scale factor
            sprite_t*     sprite;    // currently bound sprite (nullable)
            font_t*       font;      // currently bound font (nullable)
        };

        // ============================================================================
        // Draw Context
        // ============================================================================

        struct context_t
        {
            sprite_context_t* sprite_ctx;          // sprite context, owned by the caller, must outlive this context
            font_context_t*   font_ctx;            // font context, owned by the caller, must outlive this context
            draw_state_t*     state_stack;         // stack of draw states, grows upwards
            u32               state_top;           // index of the current top of the stack (0 when stack is empty)
            u32               state_capacity;      // capacity of the state stack (number of draw states it can hold)
            draw_state_t*     state;               // convenience pointer to current state
            framedescr_t      framebuffer_descr;   // description of the current framebuffer (width, height, format)
            void*             framebuffer_pixels;  // pointer to the current framebuffer pixel data
        };

        // ============================================================================
        // Sprite System
        // ============================================================================

        struct sprite_t
        {
            u16            width;
            u16            height;
            u16            format;
            u16            reserved;
            u32            pixel_data_size;
            u32            alpha_data_size;
            const void*    pixel_data;
            const void*    alpha_data;
            const color_t* palette_data;  // always u32[256] RGBA8888 palette, used only if pixel_format is indexed
        };

        struct sprite_context_t
        {
            sprite_t* sprites;
            u32       count;
            u32       reserved;  // padding to make sizeof(sprite_context_t) a multiple of 8
        };

        // ============================================================================
        // Font System
        // ============================================================================

        struct glyph_t
        {
            i16 advance_x;  // how much to move the pen horizontally to the next character after drawing this one
            i16 bearing_x;  // horizontal distance from the pen position to the left edge of the glyph bitmap
            i16 bearing_y;  // vertical distance from the pen position to the top edge of the glyph bitmap (can be negative)
            u16 width;      // width of the glyph bitmap in pixels
            u16 height;     // height of the glyph bitmap in pixels
        };

        // sizeof(font_t) must be a multiple of 8 to ensure alignment
        struct font_t
        {
            glyph_t*   glyphs;    // array of glyphs, indexed by glyph index (not ASCII code)
            const u8** bitmaps;   // alpha or coverage bitmap
            u8         map[256];  // maps ASCII character codes to glyph indices in the glyphs array, or 0xFF if the character is not supported
            i16        ascent;    // distance from baseline to top of font
            i16        descent;   // distance from baseline to bottom of font (negative value)
            i16        line_gap;  // distance from bottom of one line to top of next line (can be negative)
            i16        reserved;  // padding to make sizeof(font_t) a multiple of 8
        };

        // sizeof(font_context_t) must be a multiple of 8 to ensure alignment
        struct font_context_t
        {
            font_t* fonts;
            u32     count;
            u32     reserved;
        };

    }  // namespace ngx2
}  // namespace ncore

#endif  /// __CGX2_TYPES_H__
