#include "ccore/c_target.h"
#include "ccore/c_allocator.h"
#include "ccore/c_math.h"
#include "ccore/c_memory.h"

#include "cgx2/c_draw.h"
#include "cgx2/c_sprite.h"
#include "cgx2/c_font.h"

#include <cmath>

namespace ncore
{
    namespace ngx2
    {
        // ============================================================================
        // Framebuffer helper functions
        // ============================================================================

        inline color_t* rgba8888(void* pixels) { return static_cast<color_t*>(pixels); }  // RGBA8888 pixel buffer
        inline u16*     rgb565(void* pixels) { return static_cast<u16*>(pixels); }        // RGB565 pixel buffer

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
        static void s_put_pixel(context_t& ctx, i32 x, i32 y)
        {
            // scissor is initialized to the frame-buffer bounds
            const rect_t& sc = ctx.state->scissor;
            if (x < sc.x || x >= sc.x + sc.w || y < sc.y || y >= sc.y + sc.h)
                return;

            const color_t& src = ctx.state->color;

            void*    pixels = ctx.framebuffer_pixels;
            color_t& dst    = rgba8888(pixels)[x + y * (i32)ctx.framebuffer_descr.width];

            const u32 sa = ctx.state->sa + 1;  // add 1 to convert from 0..255 to 1..256 for easier math
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
        static void s_hspan(context_t& ctx, i32 x0, i32 x1, i32 y)
        {
            if (x0 > x1)
            {
                i32 t = x0;
                x0    = x1;
                x1    = t;
            }

            const rect_t& sc = ctx.state->scissor;
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

            const color_t& src = ctx.state->color;
            const u32      sa  = ctx.state->sa + 1;  // keep math identical to s_put_pixel

            void*    pixels = ctx.framebuffer_pixels;
            color_t* dst    = &rgba8888(pixels)[x0 + y * (i32)ctx.framebuffer_descr.width];
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
        static void s_vspan(context_t& ctx, i32 x0, i32 x1, i32 y)
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
        static void s_bresenham(context_t& ctx, i32 x0, i32 y0, i32 x1, i32 y1)
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

        // ============================================================================
        // Drawing 
        // ============================================================================

        void draw_pixel(context_t& ctx, i32 x, i32 y) { s_put_pixel(ctx, x, y); }
        void draw_line(context_t& ctx, i32 x0, i32 y0, i32 x1, i32 y1) { s_bresenham(ctx, x0, y0, x1, y1); }
        void draw_hline(context_t& ctx, i32 x0, i32 x1, i32 y) { s_hspan(ctx, x0, x1, y); }
        void draw_vline(context_t& ctx, i32 x, i32 y0, i32 y1) { s_vspan(ctx, y0, y1, x); }

        void draw_hdline(context_t& ctx, i32 x0, i32 x1, i32 y, u16 dash1, u16 dash2)
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

        void draw_vdline(context_t& ctx, i32 x, i32 y0, i32 y1, u16 dash1, u16 dash2)
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

        void draw_dline(context_t& ctx, i32 x0, i32 y0, i32 x1, i32 y1, u16 dash1, u16 dash2)
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

        void draw_arc(context_t& ctx, i32 cx, i32 cy, i32 radius, f32 start_angle, f32 end_angle)
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

        void draw_circle(context_t& ctx, i32 cx, i32 cy, i32 radius)
        {
            if (ctx.state->fill)
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

        void draw_ellipse(context_t& ctx, i32 cx, i32 cy, i32 rx, i32 ry)
        {
            if (ctx.state->fill)
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

        void draw_rectangle(context_t& ctx, i32 x, i32 y, i32 w, i32 h)
        {
            if (ctx.state->fill)
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

        void draw_sprite(context_t& ctx, i32 x, i32 y)
        {
            if (!ctx.state->sprite)
                return;

            const sprite_t& sprite    = *ctx.state->sprite;
            void*           fb_pixels = ctx.framebuffer_pixels;
            const rect_t&   sc        = ctx.state->scissor;

            const i32 fb_w = (i32)ctx.framebuffer_descr.width;
            const i32 fb_h = (i32)ctx.framebuffer_descr.height;

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

            const u32 global_alpha = (u32)ctx.state->blend_alpha + 1u;

            color_t* dst_base = rgba8888(fb_pixels);
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

        void draw_text(context_t& ctx, i32 x, i32 y, const char* text)
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
