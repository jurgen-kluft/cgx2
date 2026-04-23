#ifndef __CGX2_FONT_H__
#define __CGX2_FONT_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
    #pragma once
#endif

namespace ncore
{
    namespace ngx2
    {
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

        // ==========================================================================
        // Fonts / Text
        // ==========================================================================

        font_context_t* new_font_context(const void* binary_data, u32 binary_size);
        font_t*         get_font(font_context_t* ctx, u32 font_id);

    }  // namespace ngx2
}  // namespace ncore

#endif  /// __CGX2_FONT_H__
