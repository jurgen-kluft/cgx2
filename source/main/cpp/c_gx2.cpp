#include "ccore/c_target.h"
#include "ccore/c_allocator.h"
#include "ccore/c_math.h"
#include "ccore/c_memory.h"

#include "cgx2/c_gx2.h"

#include <cmath>

namespace ncore
{
    namespace ngx2
    {
        // ============================================================================
        // Framebuffer
        // ============================================================================
        inline color_t* rgba8888(framebuffer_t* fb) { return static_cast<color_t*>(fb->pixels); }  // RGBA8888 pixel buffer
        inline u16*     rgb565(framebuffer_t* fb) { return static_cast<u16*>(fb->pixels); }        // RGB565 pixel buffer

        // ============================================================================
        // Blend + Draw State
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
        // Context
        // ============================================================================

        struct context_t
        {
            sprite_context_t* sprite_ctx;      // sprite context, owned by the caller, must outlive this context
            font_context_t*   font_ctx;        // font context, owned by the caller, must outlive this context
            draw_state_t*     state_stack;     // stack of draw states, grows upwards
            u32               state_top;       // index of the current top of the stack (0 when stack is empty)
            u32               state_capacity;  // capacity of the state stack (number of draw states it can hold)
            draw_state_t*     state;           // convenience pointer to current state
            framebuffer_t*    framebuffer;     // active framebuffer
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

        // ============================================================================
        // Geometry & Utility Functions
        // ============================================================================

        // Bresenham's line algorithm : https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
        static void s_draw_line(u32* pixels, u32 width, u32 height, i32 x0, i32 y0, i32 x1, i32 y1, u32 color)
        {
            const i32 dx  = abs(x1 - x0);
            const i32 sx  = x0 < x1 ? 1 : -1;
            const i32 dy  = -abs(y1 - y0);
            const i32 sy  = y0 < y1 ? 1 : -1;
            i32       err = dx + dy;
            i32       e2;

            while (true)
            {
                if (0 <= x0 && x0 < (i32)width && 0 <= y0 && y0 < (i32)height)
                {
                    pixels[x0 + y0 * width] = color;
                }

                if (x0 == x1 && y0 == y1)
                {
                    break;
                }

                e2 = 2 * err;
                if (e2 >= dy)
                {
                    err += dy;
                    x0 += sx;
                }
                if (e2 <= dx)
                {
                    err += dx;
                    y0 += sy;
                }
            }
        }

        // ============================================================================
        // Internal Drawing Helpers
        // ============================================================================

        // Write a single pixel with scissor clipping and alpha blending.
        static void s_put_pixel(context_t* ctx, i32 x, i32 y)
        {
            // scissor is initialized to the frame-buffer bounds
            const rect_t& sc = ctx->state->scissor;
            if (x < sc.x || x >= sc.x + sc.w || y < sc.y || y >= sc.y + sc.h)
                return;

            const color_t& src = ctx->state->color;

            framebuffer_t* fb  = ctx->framebuffer;
            color_t&       dst = rgba8888(fb)[x + y * (i32)fb->width];

            const u32 sa = ctx->state->sa + 1;  // add 1 to convert from 0..255 to 1..256 for easier math
            if (sa == 256u)
            {
                dst = src;
            }
            else
            {
                const u32 inv = 256u - sa;
                const u8  rr  = (u8)(((u32)src.r * sa + (u32)dst.r * inv) >> 8);
                const u8  rg  = (u8)(((u32)src.g * sa + (u32)dst.g * inv) >> 8);
                const u8  rb  = (u8)(((u32)src.b * sa + (u32)dst.b * inv) >> 8);
                const u8  ra  = (u8)(((u32)src.a * sa + (u32)dst.a * inv) >> 8);
                dst           = {rr, rg, rb, ra};
            }
        }

        // Horizontal pixel span (inclusive endpoints).
        static void s_hspan(context_t* ctx, i32 x0, i32 x1, i32 y)
        {
            if (x0 > x1)
            {
                i32 t = x0;
                x0    = x1;
                x1    = t;
            }

            const rect_t& sc = ctx->state->scissor;
            if (y < sc.y || y >= sc.y + sc.h)
                return;

            if (x0 < sc.x)
                x0 = sc.x;
            if (x1 >= sc.x + sc.w)
                x1 = sc.x + sc.w - 1;
            if (x0 < 0)
                x0 = 0;

            if (x0 > x1)
                return;

            const color_t& src = ctx->state->color;
            const u32      sa  = ctx->state->sa + 1;  // keep math identical to s_put_pixel

            framebuffer_t* fb  = ctx->framebuffer;
            color_t*       dst = &rgba8888(fb)[x0 + y * (i32)fb->width];
            if (sa == 256u)
            {
                for (i32 x = x0; x <= x1; ++x, ++dst)
                    *dst = src;
            }
            else
            {
                const u32 inv = 256u - sa;
                for (i32 x = x0; x <= x1; ++x, ++dst)
                {
                    const u8 rr = (u8)(((u32)src.r * sa + (u32)dst->r * inv) >> 8);
                    const u8 rg = (u8)(((u32)src.g * sa + (u32)dst->g * inv) >> 8);
                    const u8 rb = (u8)(((u32)src.b * sa + (u32)dst->b * inv) >> 8);
                    const u8 ra = (u8)(((u32)src.a * sa + (u32)dst->a * inv) >> 8);
                    *dst        = {rr, rg, rb, ra};
                }
            }
        }

        // Vertical pixel span (inclusive endpoints).
        static void s_vspan(context_t* ctx, i32 x0, i32 x1, i32 y)
        {
            if (x0 > x1)
            {
                i32 t = x0;
                x0    = x1;
                x1    = t;
            }
        }

        // Bresenham line from (x0,y0) to (x1,y1) via s_put_pixel; if half_thick > 0 each
        // raster point is replaced by a filled disc of that radius.
        static void s_bresenham(context_t* ctx, i32 x0, i32 y0, i32 x1, i32 y1)
        {
            i32 dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
            i32 dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
            i32 err = dx + dy;

            while (true)
            {
                s_put_pixel(ctx, x0, y0);
                if (x0 == x1 && y0 == y1)
                    break;

                const i32 e2 = 2 * err;
                if (e2 >= dy)
                {
                    err += dy;
                    x0 += sx;
                }
                if (e2 <= dx)
                {
                    err += dx;
                    y0 += sy;
                }
            }
        }

        // =============================================================================
        // Framebuffer
        // ============================================================================

        framebuffer_t* new_framebuffer(alloc_t* alloc, u16 width, u16 height, image_format_t format)
        {
            framebuffer_t* fb = (framebuffer_t*)alloc->allocate(sizeof(framebuffer_t));
            if (fb)
            {
                fb->width  = width;
                fb->height = height;
                fb->format = format;

                const u32 pixel_count = (u32)width * (u32)height;
                switch (format)
                {
                    case FORMAT_RGBA8888: fb->pixels = g_allocate_array<color_t>(alloc, pixel_count); break;
                    case FORMAT_RGB565: fb->pixels = g_allocate_array<u16>(alloc, pixel_count); break;
                    default: alloc->deallocate(fb); return nullptr;
                }
            }
            return fb;
        }

        void release_framebuffer(framebuffer_t* framebuffer, alloc_t* allocator)
        {
            if (framebuffer)
            {
                g_deallocate_array(allocator, framebuffer->pixels);
                allocator->deallocate(framebuffer);
            }
        }

        void clear_full_framebuffer(context_t* ctx, color_t color)
        {
            framebuffer_t* fb = ctx->framebuffer;
            if (fb->format == FORMAT_RGBA8888)
            {
                const u32 pixel_count = (u32)fb->width * (u32)fb->height;
                color_t*  pixels      = rgba8888(fb);
                for (u32 i = 0; i < pixel_count; ++i)
                    pixels[i] = color;
            }
            else if (fb->format == FORMAT_RGB565)
            {
                // Convert RGBA8888 to RGB565 with simple bit-shift truncation (no dithering or error diffusion).
                const u16 r5 = color.r >> 3;
                const u16 g6 = color.g >> 2;
                const u16 b5 = color.b >> 3;
                const u16 c  = (r5 << 11) | (g6 << 5) | b5;

                const u32 pixel_count = (u32)fb->width * (u32)fb->height;
                u16*      pixels      = rgb565(fb);
                for (u32 i = 0; i < pixel_count; ++i)
                    pixels[i] = c;
            }
        }

        void quantize_framebuffer(context_t* ctx, framebuffer_t* target_framebuffer)
        {
            // Sanity check
            if (ctx->framebuffer == nullptr || target_framebuffer == nullptr)
                return;
            // Verify that source and target framebuffers have the same dimensions
            if (ctx->framebuffer->width != target_framebuffer->width || ctx->framebuffer->height != target_framebuffer->height)
                return;

            framebuffer_t* src = ctx->framebuffer;
            if (src->format == FORMAT_RGBA8888)
            {
                if (target_framebuffer->format == FORMAT_RGB565)
                {
                    const u32 pixel_count = (u32)src->width * (u32)src->height;
                    color_t*  src_pixels  = rgba8888(src);
                    u16*      dst_pixels  = rgb565(target_framebuffer);
                    for (u32 i = 0; i < pixel_count; ++i)
                    {
                        const color_t& c  = src_pixels[i];
                        const u16      r5 = c.r >> 3;
                        const u16      g6 = c.g >> 2;
                        const u16      b5 = c.b >> 3;
                        dst_pixels[i]     = (r5 << 11) | (g6 << 5) | b5;
                    }
                }
            }
        }

        void copy_cell_data(const framebuffer_t* fb, u16 cell_x, u16 cell_y, u8* cell_data, u16& inout_cell_width, u16& inout_cell_height, u32& inout_cell_size_in_bytes)
        {
            const u16 cell_w = inout_cell_width;
            const u16 cell_h = inout_cell_height;

            const u16 x0 = cell_x * cell_w;
            const u16 y0 = cell_y * cell_h;

            // clamp the cell dimensions to the framebuffer bounds
            const u16 cw = ((x0 + cell_w) < fb->width) ? cell_w : (fb->width - x0);
            const u16 ch = ((y0 + cell_h) < fb->height) ? cell_h : (fb->height - y0);

            const u32 bytes_per_pixel   = (fb->format == FORMAT_RGBA8888) ? 4 : 2;
            const u32 row_size_in_bytes = cw * bytes_per_pixel;

            u8* dst = cell_data;
            for (u16 y = 0; y < ch; y++)
            {
                const u8* src_row = (const u8*)fb->pixels + ((y0 + y) * fb->width + x0) * bytes_per_pixel;
                g_memcpy(dst, src_row, row_size_in_bytes);
                dst += row_size_in_bytes;
            }

            inout_cell_width         = cw;
            inout_cell_height        = ch;
            inout_cell_size_in_bytes = row_size_in_bytes * ch;
        }

        // ============================================================================
        // Context
        // ============================================================================

        context_t* new_context(alloc_t* alloc, font_context_t* font_ctx, sprite_context_t* sprite_ctx)
        {
            context_t* ctx = (context_t*)alloc->allocate(sizeof(context_t));
            if (ctx)
            {
                ctx->sprite_ctx  = sprite_ctx;
                ctx->font_ctx    = font_ctx;
                ctx->state_stack = (draw_state_t*)alloc->allocate(sizeof(draw_state_t) * 16);  // fixed capacity of 16 states for simplicity
                ctx->state_top   = 1;
                ctx->state       = &ctx->state_stack[0];
                ctx->framebuffer = nullptr;

                // Initialize the default draw state
                ctx->state->color    = {255, 255, 255, 255};
                ctx->state->blend    = {255, 0};
                ctx->state->sa       = 255;
                ctx->state->scissor  = {0, 0, ctx->framebuffer->width, ctx->framebuffer->height};
                ctx->state->fill     = 0;
                ctx->state->rotation = 0.0f;
                ctx->state->scale_x  = 1.0f;
                ctx->state->scale_y  = 1.0f;
                ctx->state->sprite   = get_sprite(ctx->sprite_ctx, 0);
                ctx->state->font     = get_font(ctx->font_ctx, 0);
            }

            return ctx;
        }

        void release_context(context_t* ctx, alloc_t* allocator)
        {
            allocator->deallocate(ctx->state_stack);
            allocator->deallocate(ctx);
        }

        void begin_frame(context_t* ctx, framebuffer_t* framebuffer)
        {
            ctx->framebuffer            = framebuffer;
            ctx->state_top              = 1;
            ctx->state_stack[0].scissor = {0, 0, framebuffer->width, framebuffer->height};
        }

        void end_frame(context_t* ctx) { ctx->framebuffer = nullptr; }

        // --------------------------------------------------------------------------
        // State Stack
        // ------------------------------------------------------------------------

        void push_state(context_t* ctx)
        {
            if (ctx->state_top < ctx->state_capacity)
            {
                ctx->state_stack[ctx->state_top] = *ctx->state;
                ctx->state                       = &ctx->state_stack[ctx->state_top];

                // When pushing a new state, we copy the previous state as the starting point for the new state.
                // This way the caller can modify only the properties they want to change and leave the rest unchanged.
                ctx->state_stack[ctx->state_top] = ctx->state_stack[ctx->state_top - 1];

                ctx->state_top++;
            }
        }
        void pop_state(context_t* ctx)
        {
            if (ctx->state_top > 0)
            {
                ctx->state_top--;
                ctx->state = &ctx->state_stack[ctx->state_top - 1];
            }
        }

        // =============================================================================
        // State Setters
        // ============================================================================

        void set_color(context_t* ctx, color_t color)
        {
            ctx->state->color = color;
            ctx->state->sa    = (color.a * ctx->state->blend.alpha) >> 8;
        }

        void set_blend_state(context_t* ctx, blend_state_t blend)
        {
            ctx->state->blend = blend;
            ctx->state->sa    = (ctx->state->color.a * blend.alpha) >> 8;
        }

        void set_scissor_rect(context_t* ctx, rect_t rect) { ctx->state->scissor = rect; }
        void set_fill(context_t* ctx, u8 enable) { ctx->state->fill = enable ? 1 : 0; }
        void set_rotation(context_t* ctx, f32 angle) { ctx->state->rotation = angle; }

        void set_scale(context_t* ctx, f32 sx, f32 sy)
        {
            ctx->state->scale_x = sx;
            ctx->state->scale_y = sy;
        }

        void set_sprite(context_t* ctx, sprite_t* sprite) { ctx->state->sprite = sprite; }
        void set_font(context_t* ctx, font_t* font) { ctx->state->font = font; }

        // ============================================================================
        // Drawing Primitives
        // ============================================================================

        void draw_pixel(context_t* ctx, i32 x, i32 y) { s_put_pixel(ctx, x, y); }
        void draw_line(context_t* ctx, i32 x0, i32 y0, i32 x1, i32 y1) { s_bresenham(ctx, x0, y0, x1, y1); }
        void draw_hline(context_t* ctx, i32 x0, i32 x1, i32 y) { s_hspan(ctx, x0, x1, y); }
        void draw_vline(context_t* ctx, i32 x, i32 y0, i32 y1) { s_vspan(ctx, y0, y1, x); }

        void draw_hdline(context_t* ctx, i32 x0, i32 x1, i32 y, u16 dash1, u16 dash2)
        {
            if (x0 > x1)
            {
                i32 t = x0;
                x0    = x1;
                x1    = t;
            }
            u16 period = dash1 + dash2;
            if (period == 0)
                period = 1;
            i32 i = 0;
            for (i32 x = x0; x <= x1; x++, i++)
            {
                if ((i % period) < dash1)
                    s_put_pixel(ctx, x, y);
            }
        }

        void draw_vdline(context_t* ctx, i32 x, i32 y0, i32 y1, u16 dash1, u16 dash2)
        {
            if (y0 > y1)
            {
                i32 t = y0;
                y0    = y1;
                y1    = t;
            }

            u16 period = dash1 + dash2;
            if (period == 0)
                period = 1;

            i32 i = 0;
            for (i32 y = y0; y <= y1; y++, i++)
            {
                if ((i % period) < dash1)
                    s_put_pixel(ctx, x, y);
            }
        }

        void draw_dline(context_t* ctx, i32 x0, i32 y0, i32 x1, i32 y1, u16 dash1, u16 dash2)
        {
            u16 period = dash1 + dash2;
            if (period == 0)
                period = 1;

            i32 dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
            i32 dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
            i32 err = dx + dy;
            i32 i   = 0;
            while (true)
            {
                if ((i % period) < dash1)
                {
                    s_put_pixel(ctx, x0, y0);
                }
                if (x0 == x1 && y0 == y1)
                    break;
                i32 e2 = 2 * err;
                if (e2 >= dy)
                {
                    err += dy;
                    x0 += sx;
                }
                if (e2 <= dx)
                {
                    err += dx;
                    y0 += sy;
                }
                i++;
            }
        }

        void draw_arc(context_t* ctx, i32 cx, i32 cy, i32 radius, f32 start_angle, f32 end_angle)
        {
            // Normalise so that end_angle >= start_angle.
            const f32 two_pi = 2.0f * (f32)math::PI;
            while (end_angle < start_angle)
                end_angle += two_pi;

            // Number of steps: at least one per pixel on the arc perimeter.
            i32 steps = (i32)(radius * (end_angle - start_angle)) + 2;
            if (steps < 2)
                steps = 2;
            f32 da = (end_angle - start_angle) / (f32)steps;
            for (i32 s = 0; s <= steps; s++)
            {
                f32 a = start_angle + s * da;
                i32 x = cx + (i32)roundf(cosf(a) * (f32)radius);
                i32 y = cy + (i32)roundf(sinf(a) * (f32)radius);
                s_put_pixel(ctx, x, y);
            }
        }

        void draw_circle(context_t* ctx, i32 cx, i32 cy, i32 radius)
        {
            if (ctx->state->fill)
            {
                // Scan-convert filled circle using horizontal spans.
                for (i32 dy = -radius; dy <= radius; dy++)
                {
                    i32 dx = (i32)sqrtf((f32)(radius * radius - dy * dy));
                    s_hspan(ctx, cx - dx, cx + dx, cy + dy);
                }
            }
            else
            {
                // Midpoint circle algorithm (8-way symmetry).
                i32 x = radius, y = 0, err = 0;
                while (x >= y)
                {
                    s_put_pixel(ctx, cx + x, cy + y);
                    s_put_pixel(ctx, cx + y, cy + x);
                    s_put_pixel(ctx, cx - y, cy + x);
                    s_put_pixel(ctx, cx - x, cy + y);
                    s_put_pixel(ctx, cx - x, cy - y);
                    s_put_pixel(ctx, cx - y, cy - x);
                    s_put_pixel(ctx, cx + y, cy - x);
                    s_put_pixel(ctx, cx + x, cy - y);
                    y++;
                    if (err <= 0)
                        err += 2 * y + 1;
                    else
                    {
                        x--;
                        err += 2 * (y - x) + 1;
                    }
                }
            }
        }

        void draw_ellipse(context_t* ctx, i32 cx, i32 cy, i32 rx, i32 ry)
        {
            if (ctx->state->fill)
            {
                // Scan-convert filled ellipse using horizontal spans.
                for (i32 dy = -ry; dy <= ry; dy++)
                {
                    f32 t = 1.0f - ((f32)dy / (f32)ry) * ((f32)dy / (f32)ry);
                    if (t < 0.0f)
                        t = 0.0f;
                    i32 dx = (i32)((f32)rx * sqrtf(t));
                    s_hspan(ctx, cx - dx, cx + dx, cy + dy);
                }
            }
            else
            {
                // Midpoint ellipse algorithm, two-region approach.
                i64 rx2 = (i64)rx * rx, ry2 = (i64)ry * ry;
                i64 x = 0, y = ry;
                i64 dx = 2 * ry2 * x, dy = 2 * rx2 * y;

                // Region 1
                i64 d1 = ry2 - rx2 * ry + (rx2 + 2) / 4;
                while (dx < dy)
                {
                    s_put_pixel(ctx, cx + (i32)x, cy + (i32)y);
                    s_put_pixel(ctx, cx - (i32)x, cy + (i32)y);
                    s_put_pixel(ctx, cx + (i32)x, cy - (i32)y);
                    s_put_pixel(ctx, cx - (i32)x, cy - (i32)y);
                    if (d1 < 0)
                    {
                        x++;
                        dx += 2 * ry2;
                        d1 += dx + ry2;
                    }
                    else
                    {
                        x++;
                        y--;
                        dx += 2 * ry2;
                        dy -= 2 * rx2;
                        d1 += dx - dy + ry2;
                    }
                }

                // Region 2
                i64 d2 = ry2 * (x * x + x) + rx2 * (y * y - y) - rx2 * ry2 + (ry2 + 2) / 4;
                while (y >= 0)
                {
                    s_put_pixel(ctx, cx + (i32)x, cy + (i32)y);
                    s_put_pixel(ctx, cx - (i32)x, cy + (i32)y);
                    s_put_pixel(ctx, cx + (i32)x, cy - (i32)y);
                    s_put_pixel(ctx, cx - (i32)x, cy - (i32)y);
                    if (d2 > 0)
                    {
                        y--;
                        dy -= 2 * rx2;
                        d2 += rx2 - dy;
                    }
                    else
                    {
                        y--;
                        x++;
                        dx += 2 * ry2;
                        dy -= 2 * rx2;
                        d2 += dx - dy + rx2;
                    }
                }
            }
        }

        void draw_rectangle(context_t* ctx, i32 x, i32 y, i32 w, i32 h)
        {
            if (ctx->state->fill)
            {
                for (i32 row = y; row < y + h; row++)
                    s_hspan(ctx, x, x + w - 1, row);
            }
            else
            {
                // Top and bottom edges
                for (i32 px = x; px < x + w; px++)
                {
                    s_put_pixel(ctx, px, y);
                    s_put_pixel(ctx, px, y + h - 1);
                }

                // Left and right edges (excluding corner pixels already drawn above)
                for (i32 py = y + 1; py < y + h - 1; py++)
                {
                    s_put_pixel(ctx, x, py);
                    s_put_pixel(ctx, x + w - 1, py);
                }
            }
        }

        // ============================================================================
        // Sprite System
        // ============================================================================

        sprite_context_t* new_sprite_context(const void* binary_data, u32 binary_size)
        {
            // Pointers replaced by u64 offsets from start of binary.
            // The format of the binary data:
            //   - [u64 offset to sprite 0 pixel data from start of binary]
            //   - [u32 sprite_count]
            //   - sprite_t[sprite_count]

            sprite_context_t* ctx = (sprite_context_t*)binary_data;
            ctx->sprites          = (sprite_t*)((const u8*)binary_data + (u64)ctx->sprites);

            for (u32 i = 0; i < ctx->count; i++)
            {
                sprite_t* sprite   = &ctx->sprites[i];
                sprite->pixel_data = ((const u8*)binary_data + (u64)sprite->pixel_data);
                if (sprite->alpha_data != 0)
                    sprite->alpha_data = ((const u8*)binary_data + (u64)sprite->alpha_data);
                if (sprite->palette_data != 0)
                    sprite->palette_data = (const color_t*)((const u8*)binary_data + (u64)sprite->palette_data);
            }

            return ctx;
        }

        sprite_t* get_sprite(sprite_context_t* ctx, u32 sprite_id)
        {
            if (sprite_id >= ctx->count)
                return nullptr;
            return &ctx->sprites[sprite_id];
        }

        void draw_sprite(context_t* ctx, i32 x, i32 y)
        {
            if (!ctx->state->sprite)
                return;

            const sprite_t& sprite = *ctx->state->sprite;
            framebuffer_t*  fb     = ctx->framebuffer;
            const rect_t&   sc     = ctx->state->scissor;

            const i32 fb_w = (i32)fb->width;
            const i32 fb_h = (i32)fb->height;

            const i32 sprite_x0 = x;
            const i32 sprite_y0 = y;
            const i32 sprite_x1 = x + (i32)sprite.width;
            const i32 sprite_y1 = y + (i32)sprite.height;

            i32 draw_x0 = sprite_x0;
            i32 draw_y0 = sprite_y0;
            i32 draw_x1 = sprite_x1;
            i32 draw_y1 = sprite_y1;

            if (draw_x0 < sc.x)
                draw_x0 = sc.x;
            if (draw_y0 < sc.y)
                draw_y0 = sc.y;
            if (draw_x1 > sc.x + sc.w)
                draw_x1 = sc.x + sc.w;
            if (draw_y1 > sc.y + sc.h)
                draw_y1 = sc.y + sc.h;

            if (draw_x0 < 0)
                draw_x0 = 0;
            if (draw_y0 < 0)
                draw_y0 = 0;
            if (draw_x1 > fb_w)
                draw_x1 = fb_w;
            if (draw_y1 > fb_h)
                draw_y1 = fb_h;

            if (draw_x0 >= draw_x1 || draw_y0 >= draw_y1)
                return;

            const i32 src_x0 = draw_x0 - sprite_x0;
            const i32 src_y0 = draw_y0 - sprite_y0;
            const i32 span_w = draw_x1 - draw_x0;

            const u32 global_alpha = (u32)ctx->state->blend.alpha + 1u;

            color_t* dst_base = rgba8888(fb);
            for (i32 j = 0; j < draw_y1 - draw_y0; ++j)
            {
                // TODO, a sprite might have a different pixel format as well as a separate alpha map

                const u32* src = &((const u32*)sprite.pixel_data)[(src_y0 + j) * (i32)sprite.width + src_x0];
                color_t*   dst = &dst_base[draw_x0 + (draw_y0 + j) * fb_w];

                for (i32 i = 0; i < span_w; ++i, ++src, ++dst)
                {
                    const u32 pixel = *src;
                    const u8  pa    = (u8)((pixel >> 24) & 0xFF);
                    if (pa == 0u)
                        continue;

                    const color_t src_color = {(u8)((pixel >> 16) & 0xFF), (u8)((pixel >> 8) & 0xFF), (u8)(pixel & 0xFF), pa};

                    const u32 sa = (((u32)pa * global_alpha) >> 8) + 1u;
                    if (sa == 255u)
                    {
                        *dst = src_color;
                    }
                    else
                    {
                        const u32 inv = (u8)(255u - sa) + 1u;
                        const u8  rr  = (u8)(((u32)src_color.r * sa + (u32)dst->r * inv) >> 8);
                        const u8  rg  = (u8)(((u32)src_color.g * sa + (u32)dst->g * inv) >> 8);
                        const u8  rb  = (u8)(((u32)src_color.b * sa + (u32)dst->b * inv) >> 8);
                        const u8  ra  = (u8)(((u32)src_color.a * sa + (u32)dst->a * inv) >> 8);
                        *dst          = {rr, rg, rb, ra};
                    }
                }
            }
        }

        // ============================================================================
        // Font System
        // ============================================================================

        font_context_t* new_font_context(const void* binary_data, u32 binary_size)
        {
            // Pointers replaced by u64 offsets from start of binary.
            // The format of the binary data:
            //   - [u64 offset to font 0 glyph data from start of binary]
            //   - [u32 font_count]
            //   - [u32 reserved]
            //   - font_t[font_count]

            font_context_t* ctx = (font_context_t*)binary_data;
            ctx->fonts          = (font_t*)((const u8*)binary_data + (u64)ctx->fonts);

            for (u32 i = 0; i < ctx->count; i++)
            {
                font_t* font  = &ctx->fonts[i];
                font->glyphs  = (glyph_t*)((const u8*)binary_data + (u64)font->glyphs);
                font->bitmaps = (const u8**)((const u8*)binary_data + (u64)font->bitmaps);
            }

            return ctx;
        }

        font_t* get_font(font_context_t* ctx, u32 font_id)
        {
            if (font_id >= ctx->count)
                return nullptr;
            return &ctx->fonts[font_id];
        }

        void draw_text(context_t* ctx, i32 x, i32 y, const char* text)
        {
            // Drawing text is a bit more complex because of font metrics, but the basic idea is:
            // - User requests to draw text at a certain position (top left corner x,y) with the
            //   current font and state.
            // For each character in the text:
            //   - Look up the glyph for the character in the font's map.
            //   - If the glyph exists, draw its bitmap at the appropriate position, applying the
            //     glyph's advance and bearing to position the next character correctly.
            //   - If the glyph does not exist, skip it or draw a placeholder like ' ' or '?'.
            // - The glyph bitmap is drawn using the current color and alpha, and should be blended
            //   with the framebuffer if the blend alpha is less than 255.

            // TODO Implement text drawing using the font context and glyph metrics.
            //      This will involve iterating over each character in the text, looking up the
            //      corresponding glyph in the font, and then drawing the glyph's bitmap onto the
            //      framebuffer at the correct position, applying the current color and blend state
            //      for alpha blending. The positioning of each glyph will need to take into account
            //      the advance and bearing values from the glyph metrics to ensure proper spacing
            //      between characters.
        }

    }  // namespace ngx2
}  // namespace ncore
