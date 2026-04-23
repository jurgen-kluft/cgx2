#ifndef __CGX2_DRAW_H__
#define __CGX2_DRAW_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
    #pragma once
#endif

#include "cgx2/c_draw_context.h"

namespace ncore
{
    namespace ngx2
    {
        // ============================================================================
        // Drawing 
        // ============================================================================

        void draw_pixel(context_t& ctx, i32 x, i32 y);
        void draw_line(context_t& ctx, i32 x0, i32 y0, i32 x1, i32 y1);
        void draw_hline(context_t& ctx, i32 x0, i32 x1, i32 y);
        void draw_vline(context_t& ctx, i32 x, i32 y0, i32 y1);
        void draw_hdline(context_t& ctx, i32 x0, i32 x1, i32 y, u16 dash1, u16 dash2);
        void draw_vdline(context_t& ctx, i32 x, i32 y0, i32 y1, u16 dash1, u16 dash2);
        void draw_dline(context_t& ctx, i32 x0, i32 y0, i32 x1, i32 y1, u16 dash1, u16 dash2);
        void draw_arc(context_t& ctx, i32 x, i32 y, i32 radius, f32 start_angle, f32 end_angle);
        void draw_circle(context_t& ctx, i32 x, i32 y, i32 radius);
        void draw_ellipse(context_t& ctx, i32 x, i32 y, i32 rx, i32 ry);
        void draw_rectangle(context_t& ctx, i32 x, i32 y, i32 w, i32 h);
        void draw_sprite(context_t& ctx, i32 x, i32 y);
        void draw_text(context_t& ctx, i32 x, i32 y, const char* text);

    }  // namespace ngx2
}  // namespace ncore

#endif  /// __CGX2_DRAW_H__
