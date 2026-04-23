#include "ccore/c_target.h"
#include "ccore/c_allocator.h"
#include "ccore/c_math.h"
#include "ccore/c_memory.h"

#include "cgx2/c_draw_context.h"
#include "cgx2/c_sprite.h"
#include "cgx2/c_font.h"

#include <cmath>

namespace ncore
{
    namespace ngx2
    {
        // ============================================================================
        // Context
        // ============================================================================

        void init_context(context_t& ctx, font_context_t* font_ctx, sprite_context_t* sprite_ctx)
        {
            ctx.sprite_ctx = sprite_ctx;
            ctx.font_ctx   = font_ctx;

            ctx.font_ctx           = font_ctx;
            ctx.state_top          = 1;
            ctx.state_capacity     = 32;
            ctx.state              = &ctx.state_stack[0];
            ctx.framebuffer_pixels = nullptr;

            // Initialize the default draw state
            ctx.state->color            = {255, 255, 255, 255};
            ctx.state->blend_alpha      = 255;
            ctx.state->ignore_src_alpha = 0;
            ctx.state->sa               = 255;
            ctx.state->scissor          = {0, 0, ctx.framebuffer_descr.width, ctx.framebuffer_descr.height};
            ctx.state->fill             = 0;
            ctx.state->rotation         = 0.0f;
            ctx.state->scale_x          = 1.0f;
            ctx.state->scale_y          = 1.0f;
            ctx.state->sprite           = get_sprite(ctx.sprite_ctx, 0);
            ctx.state->font             = get_font(ctx.font_ctx, 0);
        }

        void begin_frame(context_t& ctx, framedescr_t const& descr, void* pixels)
        {
            ctx.framebuffer_descr      = descr;
            ctx.framebuffer_pixels     = pixels;
            ctx.state_top              = 1;
            ctx.state_stack[0].scissor = {0, 0, descr.width, descr.height};
        }

        void end_frame(context_t& ctx) { ctx.framebuffer_pixels = nullptr; }

        // --------------------------------------------------------------------------
        // State Stack
        // ------------------------------------------------------------------------

        void push_state(context_t& ctx)
        {
            if (ctx.state_top < ctx.state_capacity)
            {
                ctx.state_stack[ctx.state_top] = *ctx.state;
                ctx.state                      = &ctx.state_stack[ctx.state_top];

                // When pushing a new state, we copy the previous state as the starting point for the new state.
                // This way the caller can modify only the properties they want to change and leave the rest unchanged.
                ctx.state_stack[ctx.state_top] = ctx.state_stack[ctx.state_top - 1];

                ctx.state_top++;
            }
        }
        void pop_state(context_t& ctx)
        {
            if (ctx.state_top > 0)
            {
                ctx.state_top--;
                ctx.state = &ctx.state_stack[ctx.state_top - 1];
            }
        }

        // =============================================================================
        // State Setters
        // ============================================================================

        void set_color(context_t& ctx, color_t color)
        {
            ctx.state->color = color;
            ctx.state->sa    = (color.a * ctx.state->blend_alpha) >> 8;
        }

        void set_blend_state(context_t& ctx, u8 blend_alpha, u8 ignore_src_alpha)
        {
            ctx.state->blend_alpha      = blend_alpha;
            ctx.state->ignore_src_alpha = ignore_src_alpha ? 1 : 0;
            ctx.state->sa               = (ctx.state->color.a * blend_alpha) >> 8;
        }

        void set_scissor_rect(context_t& ctx, rect_t rect) { ctx.state->scissor = rect; }
        void set_fill(context_t& ctx, u8 enable) { ctx.state->fill = enable ? 1 : 0; }
        void set_rotation(context_t& ctx, f32 angle) { ctx.state->rotation = angle; }

        void set_scale(context_t& ctx, f32 sx, f32 sy)
        {
            ctx.state->scale_x = sx;
            ctx.state->scale_y = sy;
        }

        void set_sprite(context_t& ctx, sprite_t* sprite) { ctx.state->sprite = sprite; }
        void set_font(context_t& ctx, font_t* font) { ctx.state->font = font; }

    }  // namespace ngx2
}  // namespace ncore
