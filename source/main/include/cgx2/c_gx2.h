#ifndef __CGX2_2D_GFX_H__
#define __CGX2_2D_GFX_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
    #pragma once
#endif

namespace ncore
{
    // ============================================================================
    // Forward-Declarations (core)
    // ============================================================================

    class alloc_t;

    namespace ngx2
    {
        // ============================================================================
        // Forward-Declarations (Opaque Types)
        // ============================================================================

        struct framebuffer_t;
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

        // ============================================================================
        // Framebuffer
        // ============================================================================
        struct framebuffer_t
        {
            u16            width;     // width in pixels
            u16            height;    // height in pixels
            image_format_t format;    // pixel format
            u16            reserved;  // reserved for future use (0)
            void*          pixels;    // pixel data
        };

        framebuffer_t* new_framebuffer(alloc_t* alloc, u16 width, u16 height, image_format_t format);
        void           release_framebuffer(framebuffer_t* framebuffer, alloc_t* allocator);
        void           clear_full_framebuffer(context_t* ctx, color_t color);
        void           quantize_framebuffer(context_t* ctx, framebuffer_t* target_framebuffer);

        // Example:
        //  u16 cell_width = 16; cell_height = 16;
        //  u32 cell_size_in_bytes = cell_width * cell_height * bytes_per_pixel(fb->format);
        //  u8 cell_data[cell_size_in_bytes];
        //  copy_cell_data(fb, cell_x, cell_y, cell_data, cell_width, cell_height, cell_size_in_bytes);
        //  Note: cell_width, cell_height, and cell_size_in_bytes are input/output parameters. 
        //        The function will update them to the actual size of the copied cell data (clamped to framebuffer bounds).
        void copy_cell_data(const framebuffer_t* fb, u16 cell_x, u16 cell_y, u8* cell_data, u16& inout_cell_width, u16& inout_cell_height, u32& inout_cell_size_in_bytes);

        // ============================================================================
        // Blending
        // ============================================================================

        struct blend_state_t
        {
            u8 alpha;             // 0..255
            u8 ignore_src_alpha;  // 0 or 1
        };

        // ============================================================================
        // Context
        // ============================================================================

        context_t* new_context(alloc_t* alloc, font_context_t* font_ctx, sprite_context_t* sprite_ctx);
        void       release_context(context_t* ctx, alloc_t* allocator);
        void       begin_frame(context_t* ctx, framebuffer_t* framebuffer);
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
