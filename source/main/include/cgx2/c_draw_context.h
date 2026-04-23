#ifndef __CGX2_CONTEXT_H__
#define __CGX2_CONTEXT_H__
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
        // Context
        // ============================================================================

        struct draw_state_t
        {
            u8        blend_alpha;       // 0..255
            u8        ignore_src_alpha;  // 0 or 1
            u8        sa;                // calculated source alpha (0..256) for blending, derived from color.a and blend.alpha
            u8        fill;              // 0 or 1
            color_t   color;             // current drawing color
            f32       rotation;          // degrees
            f32       scale_x;           // horizontal scale factor
            f32       scale_y;           // vertical scale factor
            rect_t    scissor;           // active scissor rect
            sprite_t* sprite;            // currently bound sprite (nullable)
            font_t*   font;              // currently bound font (nullable)
        };

        struct context_t
        {
            sprite_context_t* sprite_ctx;          // sprite context, owned by the caller, must outlive this context
            font_context_t*   font_ctx;            // font context, owned by the caller, must outlive this context
            draw_state_t      state_stack[32];     // stack of draw states, grows upwards
            u32               state_top;           // index of the current top of the stack (0 when stack is empty)
            u32               state_capacity;      // capacity of the state stack (number of draw states it can hold)
            draw_state_t*     state;               // convenience pointer to current state
            framedescr_t      framebuffer_descr;   // description of the current framebuffer (width, height, format)
            void*             framebuffer_pixels;  // pointer to the current framebuffer pixel data
        };

        void init_context(context_t& ctx, font_context_t* font_ctx, sprite_context_t* sprite_ctx);
        void begin_frame(context_t& ctx, framedescr_t const& descr, void* pixels);
        void end_frame(context_t& ctx);

        // --------------------------------------------------------------------------
        // State Stack
        // --------------------------------------------------------------------------

        void push_state(context_t& ctx);
        void pop_state(context_t& ctx);

        // --------------------------------------------------------------------------
        // State Setters
        // --------------------------------------------------------------------------

        void set_color(context_t& ctx, color_t color);
        void set_blend_state(context_t& ctx, u8 blend_alpha, u8 ignore_src_alpha);
        void set_scissor_rect(context_t& ctx, rect_t rect);
        void set_fill(context_t& ctx, u8 enable); /* 0 or 1 */
        void set_rotation(context_t& ctx, f32 angle);
        void set_scale(context_t& ctx, f32 sx, f32 sy);
        void set_sprite(context_t& ctx, sprite_t* sprite);
        void set_font(context_t& ctx, font_t* font);

    }  // namespace ngx2
}  // namespace ncore

#endif  /// __CGX2_CONTEXT_H__
