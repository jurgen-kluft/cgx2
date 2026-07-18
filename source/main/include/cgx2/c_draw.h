#ifndef __CGX2_DRAW_H__
#define __CGX2_DRAW_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
    #pragma once
#endif

#include "cgx2/c_types.h"

namespace ncore
{
    namespace ngx2
    {
        // ============================================================================
        // Drawing
        // ============================================================================

        void draw_pixel(framebuffer_t& ctx, rect_t const& scissor, i32 x, i32 y, color_t src);
        void draw_line(framebuffer_t& ctx, rect_t const& scissor, i32 x0, i32 y0, i32 x1, i32 y1, color_t src);
        void draw_hline(framebuffer_t& ctx, rect_t const& scissor, i32 x0, i32 x1, i32 y, color_t src);
        void draw_vline(framebuffer_t& ctx, rect_t const& scissor, i32 x, i32 y0, i32 y1, color_t src);
        void draw_hdline(framebuffer_t& ctx, rect_t const& scissor, i32 x0, i32 x1, i32 y, u16 dash1, u16 dash2, color_t src);
        void draw_vdline(framebuffer_t& ctx, rect_t const& scissor, i32 x, i32 y0, i32 y1, u16 dash1, u16 dash2, color_t src);
        void draw_dline(framebuffer_t& ctx, rect_t const& scissor, i32 x0, i32 y0, i32 x1, i32 y1, u16 dash1, u16 dash2, color_t src);
        void draw_arc(framebuffer_t& ctx, rect_t const& scissor, i32 x, i32 y, i32 radius, f32 start_angle, f32 end_angle, color_t src);
        void draw_circle(framebuffer_t& ctx, rect_t const& scissor, i32 x, i32 y, i32 radius, bool fill, color_t src);
        void draw_ellipse(framebuffer_t& ctx, rect_t const& scissor, i32 x, i32 y, i32 rx, i32 ry, bool fill, color_t src);
        void draw_rectangle(framebuffer_t& ctx, rect_t const& scissor, i32 x, i32 y, i32 w, i32 h, bool fill, color_t src);

        void draw_sprite(framebuffer_t& ctx, rect_t const& scissor, sprite_t* sprite, i32 x, i32 y);
        void draw_sprite(framebuffer_t& ctx, rect_t const& scissor, sprite_t* sprite, palette_t* palette, i32 x, i32 y);

        // Draw a single line of byte-mapped glyphs with (x, y) as the pen baseline origin.
        void draw_text(framebuffer_t& ctx, rect_t const& scissor, font_t* font, i32 x, i32 y, const char* text, color_t src, float scale);

    }  // namespace ngx2
}  // namespace ncore

#endif  /// __CGX2_DRAW_H__
