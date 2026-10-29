#ifndef __CGX2_DRAW_SBR_H__
#define __CGX2_DRAW_SBR_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
    #pragma once
#endif

#include "cgx2/c_types.h"

namespace ncore
{
    namespace ngx2
    {
        struct sprite_t;
        struct palette_t;
        struct font_t;

        // A "Slice Based Renderer" is a software rasterizer that can draw into a slice of a framebuffer.
        // It is designed to be fast and efficient, and it supports a wide range of drawing operations, including
        // lines, rectangles, circles, ellipses, arcs, sprites, and text.
        // Clipping is only to screen bounds and the bounds of the slice when rendering, we don't support arbitrary
        // clipping rectangles.
        struct ctx_t
        {
            // Slice parameters
            u16* m_pixels;   // pixel data (pixel data of where the slice starts in the framebuffer)
            u16  m_width;    // width in pixels
            u16  m_height;   // height in pixels
            u16  m_offseth;  // height offset in pixels (number of lines from the top of the framebuffer to the start of the slice)
        };

        void draw_hline(ctx_t& ctx, i32 x0, i32 x1, i32 y, u32 color);
        void draw_vline(ctx_t& ctx, i32 x, i32 y0, i32 y1, u32 color);

        // Draw a dashed horizontal line from (x0, y) to (x1, y) with dash lengths dash1 and dash2.
        // Drawing dash1 pixels, will use palette[ctx.color] as the color, then skip dash2 pixels, and
        // repeat until the line is complete.
        void draw_hdline(ctx_t& ctx, i32 x0, i32 x1, i32 y, u16 dash1, u16 dash2, u32 color);
        void draw_vdline(ctx_t& ctx, i32 x, i32 y0, i32 y1, u16 dash1, u16 dash2, u32 color);

        void draw_rrect(ctx_t& ctx, i32 x, i32 y, i32 w, i32 h, i32 radius, u32 color);

        void draw_sprite(ctx_t& ctx, const sprite_t* sprite, const palette_t* palette, i32 x, i32 y, u32 color);
        void draw_sprite(ctx_t& ctx, const sprite_t* sprite, const palette_t* palette, i32 x, i32 y, i32 w, i32 h, u32 color);

        void draw_text(ctx_t& ctx, const font_t* font, u16 fontSize, i32 x, i32 y, const char* text, u32 color);

    }  // namespace ngx2
}  // namespace ncore

#endif  /// __CGX2_DRAW_SBR_H__
