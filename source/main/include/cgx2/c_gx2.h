#ifndef __CGX2_2D_GFX_H__
#define __CGX2_2D_GFX_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
    #pragma once
#endif

#include "cgx2/c_types.h"

namespace ncore
{
    // ============================================================================
    // Forward-Declarations (core)
    // ============================================================================

    class alloc_t;

    namespace ngx2
    {
        // ============================================================================
        // Context
        // ============================================================================

        context_t* new_context(alloc_t* alloc, font_context_t* font_ctx, sprite_context_t* sprite_ctx);
        void       release_context(context_t* ctx, alloc_t* allocator);
        void       begin_frame(context_t* ctx, framedescr_t const& descr, void* pixels);
        void       end_frame(context_t* ctx);

        // --------------------------------------------------------------------------
        // State Stack
        // --------------------------------------------------------------------------

        void push_state(context_t* ctx);
        void pop_state(context_t* ctx);

        // --------------------------------------------------------------------------
        // State Setters
        // --------------------------------------------------------------------------

        void set_color(context_t* ctx, color_t color);
        void set_blend_state(context_t* ctx, blend_state_t blend);
        void set_scissor_rect(context_t* ctx, rect_t rect);
        void set_fill(context_t* ctx, u8 enable); /* 0 or 1 */
        void set_rotation(context_t* ctx, f32 angle);
        void set_scale(context_t* ctx, f32 sx, f32 sy);
        void set_sprite(context_t* ctx, sprite_t* sprite);
        void set_font(context_t* ctx, font_t* font);

        // ============================================================================
        // Drawing Primitives
        // ============================================================================

        void draw_pixel(context_t* ctx, i32 x, i32 y);
        void draw_line(context_t* ctx, i32 x0, i32 y0, i32 x1, i32 y1);
        void draw_hline(context_t* ctx, i32 x0, i32 x1, i32 y);
        void draw_vline(context_t* ctx, i32 x, i32 y0, i32 y1);
        void draw_hdline(context_t* ctx, i32 x0, i32 x1, i32 y, u16 dash1, u16 dash2);
        void draw_vdline(context_t* ctx, i32 x, i32 y0, i32 y1, u16 dash1, u16 dash2);
        void draw_dline(context_t* ctx, i32 x0, i32 y0, i32 x1, i32 y1, u16 dash1, u16 dash2);
        void draw_arc(context_t* ctx, i32 x, i32 y, i32 radius, f32 start_angle, f32 end_angle);
        void draw_circle(context_t* ctx, i32 x, i32 y, i32 radius);
        void draw_ellipse(context_t* ctx, i32 x, i32 y, i32 rx, i32 ry);
        void draw_rectangle(context_t* ctx, i32 x, i32 y, i32 w, i32 h);

        // ============================================================================
        // Sprites
        // ============================================================================

        sprite_context_t* new_sprite_context(const void* binary_data, u32 binary_size);
        sprite_t*         get_sprite(sprite_context_t* ctx, u32 sprite_id);
        void              draw_sprite(context_t* ctx, i32 x, i32 y);

        // ==========================================================================
        // Fonts / Text
        // ==========================================================================

        font_context_t* new_font_context(const void* binary_data, u32 binary_size);
        font_t*         get_font(font_context_t* ctx, u32 font_id);
        void            draw_text(context_t* ctx, i32 x, i32 y, const char* text);

    }  // namespace ngx2
}  // namespace ncore

#endif  /// __CGX2_2D_GFX_H__
