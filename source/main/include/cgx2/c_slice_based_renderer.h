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
        // Slice Based Renderer (SBR) is a software rasterizer that draws directly into a slice of a framebuffer.
        // It is designed to be fast and efficient, and it supports a wide range of drawing operations, including
        // lines, rectangles, circles, ellipses, arcs, sprites, and text.
        // Clipping is only to screen bounds and the bounds of the slice when rendering, we don't support arbitrary
        // clipping rectangles.
        namespace nsbr
        {
            struct ctx_t
            {
                // Slice parameters
                u16* pixels;   // pixel data (pixel data of where the slice starts in the framebuffer)
                u16  width;    // width in pixels
                u16  height;   // height in pixels
                u16  offseth;  // height offset in pixels (number of lines from the top of the framebuffer to the start of the slice)
                // Parameters for drawing operations
                u8         color;     // current drawing color as index into palette
                u8         fill : 1;  // fill mode (0 = outline, 1 = filled)
                palette_t* palette;   // palette for indexed color
            };

            void draw_hline(ctx_t& ctx, i32 x0, i32 x1, i32 y);
            void draw_vline(ctx_t& ctx, i32 x, i32 y0, i32 y1);

            // Draw a dashed horizontal line from (x0, y) to (x1, y) with dash lengths dash1 and dash2.
            // Drawing dash1 pixels, will use palette[ctx.color] as the color, then skip dash2 pixels, and
            // repeat until the line is complete.
            void draw_hdline(ctx_t& ctx, i32 x0, i32 x1, i32 y, u16 dash1, u16 dash2);
            void draw_vdline(ctx_t& ctx, i32 x, i32 y0, i32 y1, u16 dash1, u16 dash2);

            void draw_sprite(ctx_t& ctx, sprite_t* sprite, i32 x, i32 y);
            void draw_sprite(ctx_t& ctx, sprite_t* sprite, i32 x, i32 y, i32 w, i32 h);

            void draw_text(ctx_t& ctx, font_t* font, u16 fontSize, i32 x, i32 y, const char* text);
        }  // namespace nsbr

    }  // namespace ngx2
}  // namespace ncore

#endif  /// __CGX2_DRAW_SBR_H__
