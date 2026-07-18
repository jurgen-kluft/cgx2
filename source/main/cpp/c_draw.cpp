#include "ccore/c_target.h"
#include "ccore/c_math.h"
#include "ccore/c_memory.h"

#include "cgx2/c_draw.h"
#include "cgx2/c_types.h"

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
        static void s_draw_line(u16* pixels, u32 width, u32 height, i32 x0, i32 y0, i32 x1, i32 y1, u16 color)
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
        static inline void s_put_pixel(framebuffer_t& fb, i32 x, i32 y, color_t src)
        {
            u16* pixels                         = (u16*)fb.pixels;
            pixels[x + y * (i32)fb.descr.width] = src;
        }

        // Horizontal pixel span (inclusive endpoints).
        static void s_hspan(framebuffer_t& fb, i32 x0, i32 x1, i32 y, color_t src)
        {
            u16* pixels = (u16*)fb.pixels;
            for (i32 x = x0; x <= x1; ++x)
            {
                pixels[x + y * (i32)fb.descr.width] = src;
            }
        }

        // Vertical pixel span (inclusive endpoints).
        static void s_vspan(framebuffer_t& fb, i32 x, i32 y0, i32 y1, color_t src)
        {
            u16* pixels = (u16*)fb.pixels;
            for (i32 y = y0; y <= y1; ++y)
            {
                pixels[x + y * (i32)fb.descr.width] = src;
            }
        }

        // Fast RGB565 Alpha Blending using bitwise manipulation
        // Alpha is 2-bit (0-3), where 0 is fully transparent and 3 is fully opaque.
        inline u16 s_blend_rgb565_a2(u16 bg, u16 fg, u8 coverage)
        {
            if (coverage == 0)
                return bg;
            if (coverage == 3)
                return fg;

            // 1. Separate all 3 background channels
            u32       bc    = (bg >> 11) & 0x1F;
            u32       fc    = (fg >> 11) & 0x1F;
            const u32 out_r = bc + (((int32_t)(fc - bc) * coverage) >> 2);

            bc              = (bg >> 5) & 0x3F;
            fc              = (fg >> 5) & 0x3F;
            const u32 out_g = bc + (((int32_t)(fc - bc) * coverage) >> 2);

            bc              = bg & 0x1F;
            fc              = fg & 0x1F;
            const u32 out_b = bc + (((int32_t)(fc - bc) * coverage) >> 2);

            // 4. Repack channels back into a single u16 RGB565 pixel
            return (out_r << 11) | (out_g << 5) | out_b;
        }

        // Fast RGB565 Alpha Blending using bitwise manipulation
        // Alpha is 4-bit (0-15), where 0 is fully transparent and 15 is fully opaque.
        inline u16 s_blend_rgb565_a4(u16 bg, u16 fg, u8 coverage)
        {
            if (coverage == 0)
                return bg;
            if (coverage == 15)
                return fg;

            // 1. Separate all 3 background channels
            u32       bc    = (bg >> 11) & 0x1F;
            u32       fc    = (fg >> 11) & 0x1F;
            const u32 out_r = bc + (((int32_t)(fc - bc) * coverage) >> 4);

            bc              = (bg >> 5) & 0x3F;
            fc              = (fg >> 5) & 0x3F;
            const u32 out_g = bc + (((int32_t)(fc - bc) * coverage) >> 4);

            // 2. Separate all 3 foreground channels
            bc              = bg & 0x1F;
            fc              = fg & 0x1F;
            const u32 out_b = bc + (((int32_t)(fc - bc) * coverage) >> 4);

            // 4. Repack channels back into a single u16 RGB565 pixel
            return (out_r << 11) | (out_g << 5) | out_b;
        }

        // Slow RGB565 Alpha Blending using bitwise manipulation
        // Alpha is 8-bit (0-255), where 0 is fully transparent and 255 is fully opaque.
        static inline color_t s_blend_rgb565_a8(color_t dst, color_t src, u8 coverage)
        {
            if (coverage == 0)
                return dst;
            if (coverage == 255)
                return src;

            const u32 inverse = 255 - coverage;

            const u32 dst_r = (dst >> 11) & 0x1f;
            const u32 src_r = (src >> 11) & 0x1f;
            const u32 out_r = (src_r * coverage + dst_r * inverse + 127) / 255;

            const u32 dst_g = (dst >> 5) & 0x3f;
            const u32 src_g = (src >> 5) & 0x3f;
            const u32 out_g = (src_g * coverage + dst_g * inverse + 127) / 255;

            const u32 dst_b = dst & 0x1f;
            const u32 src_b = src & 0x1f;
            const u32 out_b = (src_b * coverage + dst_b * inverse + 127) / 255;

            return (color_t)((out_r << 11) | (out_g << 5) | out_b);
        }

        // Bresenham line from (x0,y0) to (x1,y1) via s_put_pixel; if half_thick > 0 each
        // raster point is replaced by a filled disc of that radius.
        static void s_bresenham(framebuffer_t& fb, i32 x0, i32 y0, i32 x1, i32 y1, color_t src)
        {
            i32 dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
            i32 dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
            i32 err = dx + dy;

            while (true)
            {
                s_put_pixel(fb, x0, y0, src);
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

        enum
        {
            CLIP_LEFT   = 1,
            CLIP_RIGHT  = 2,
            CLIP_TOP    = 4,
            CLIP_BOTTOM = 8
        };

        struct clip_rect_t
        {
            i32 xmin, ymin, xmax, ymax;
        };

        static inline i32 outcode(const clip_rect_t& cr, i32 x, i32 y)
        {
            i32 code = 0;
            if (x < cr.xmin)
                code |= CLIP_LEFT;
            else if (x > cr.xmax)
                code |= CLIP_RIGHT;
            if (y < cr.ymin)
                code |= CLIP_TOP;
            else if (y > cr.ymax)
                code |= CLIP_BOTTOM;
            return code;
        };

        static bool s_clip_line(rect_t const& scissor, i32& x0, i32& y0, i32& x1, i32& y1)
        {
            if (scissor.w <= 0 || scissor.h <= 0)
                return false;

            const clip_rect_t cr = {scissor.x, scissor.y, scissor.x + scissor.w - 1, scissor.y + scissor.h - 1};

            i32 code0 = outcode(cr, x0, y0);
            i32 code1 = outcode(cr, x1, y1);
            while ((code0 | code1) != 0)
            {
                if ((code0 & code1) != 0)
                    return false;

                const i32 code = code0 != 0 ? code0 : code1;
                i32       x;
                i32       y;
                if ((code & CLIP_TOP) != 0)
                {
                    x = x0 + (i32)(((i64)x1 - x0) * ((i64)cr.ymin - y0) / ((i64)y1 - y0));
                    y = cr.ymin;
                }
                else if ((code & CLIP_BOTTOM) != 0)
                {
                    x = x0 + (i32)(((i64)x1 - x0) * ((i64)cr.ymax - y0) / ((i64)y1 - y0));
                    y = cr.ymax;
                }
                else if ((code & CLIP_RIGHT) != 0)
                {
                    y = y0 + (i32)(((i64)y1 - y0) * ((i64)cr.xmax - x0) / ((i64)x1 - x0));
                    x = cr.xmax;
                }
                else
                {
                    y = y0 + (i32)(((i64)y1 - y0) * ((i64)cr.xmin - x0) / ((i64)x1 - x0));
                    x = cr.xmin;
                }

                if (code == code0)
                {
                    x0    = x;
                    y0    = y;
                    code0 = outcode(cr, x0, y0);
                }
                else
                {
                    x1    = x;
                    y1    = y;
                    code1 = outcode(cr, x1, y1);
                }
            }
            return true;
        }

        // ============================================================================
        // Drawing
        // ============================================================================

        void draw_pixel(framebuffer_t& fb, i32 x, i32 y, color_t src) { s_put_pixel(fb, x, y, src); }
        void draw_line(framebuffer_t& fb, i32 x0, i32 y0, i32 x1, i32 y1, color_t src) { s_bresenham(fb, x0, y0, x1, y1, src); }
        void draw_hline(framebuffer_t& fb, i32 x0, i32 x1, i32 y, color_t src) { s_hspan(fb, x0, x1, y, src); }
        void draw_vline(framebuffer_t& fb, i32 x, i32 y0, i32 y1, color_t src) { s_vspan(fb, x, y0, y1, src); }

        void draw_hdline(framebuffer_t& fb, i32 x0, i32 x1, i32 y, u16 dash1, u16 dash2, color_t src)
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
                    s_put_pixel(fb, x, y, src);
            }
        }

        void draw_vdline(framebuffer_t& fb, i32 x, i32 y0, i32 y1, u16 dash1, u16 dash2, color_t src)
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
                    s_put_pixel(fb, x, y, src);
            }
        }

        void draw_dline(framebuffer_t& fb, i32 x0, i32 y0, i32 x1, i32 y1, u16 dash1, u16 dash2, color_t src)
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
                    s_put_pixel(fb, x0, y0, src);
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

        void draw_arc(framebuffer_t& fb, i32 cx, i32 cy, i32 radius, f32 start_angle, f32 end_angle, color_t src)
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
                s_put_pixel(fb, x, y, src);
            }
        }

        void draw_circle(framebuffer_t& fb, i32 cx, i32 cy, i32 radius, bool fill, color_t src)
        {
            if (fill)
            {
                // Scan-convert filled circle using horizontal spans.
                for (i32 dy = -radius; dy <= radius; dy++)
                {
                    i32 dx = (i32)sqrtf((f32)(radius * radius - dy * dy));
                    s_hspan(fb, cx - dx, cx + dx, cy + dy, src);
                }
            }
            else
            {
                // Midpoint circle algorithm (8-way symmetry).
                i32 x = radius, y = 0, err = 0;
                while (x >= y)
                {
                    s_put_pixel(fb, cx + x, cy + y, src);
                    s_put_pixel(fb, cx + y, cy + x, src);
                    s_put_pixel(fb, cx - y, cy + x, src);
                    s_put_pixel(fb, cx - x, cy + y, src);
                    s_put_pixel(fb, cx - x, cy - y, src);
                    s_put_pixel(fb, cx - y, cy - x, src);
                    s_put_pixel(fb, cx + y, cy - x, src);
                    s_put_pixel(fb, cx + x, cy - y, src);
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

        void draw_ellipse(framebuffer_t& fb, i32 cx, i32 cy, i32 rx, i32 ry, bool fill, color_t src)
        {
            if (fill)
            {
                // Scan-convert filled ellipse using horizontal spans.
                for (i32 dy = -ry; dy <= ry; dy++)
                {
                    f32 t = 1.0f - ((f32)dy / (f32)ry) * ((f32)dy / (f32)ry);
                    if (t < 0.0f)
                        t = 0.0f;
                    i32 dx = (i32)((f32)rx * sqrtf(t));
                    s_hspan(fb, cx - dx, cx + dx, cy + dy, src);
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
                    s_put_pixel(fb, cx + (i32)x, cy + (i32)y, src);
                    s_put_pixel(fb, cx - (i32)x, cy + (i32)y, src);
                    s_put_pixel(fb, cx + (i32)x, cy - (i32)y, src);
                    s_put_pixel(fb, cx - (i32)x, cy - (i32)y, src);
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
                    s_put_pixel(fb, cx + (i32)x, cy + (i32)y, src);
                    s_put_pixel(fb, cx - (i32)x, cy + (i32)y, src);
                    s_put_pixel(fb, cx + (i32)x, cy - (i32)y, src);
                    s_put_pixel(fb, cx - (i32)x, cy - (i32)y, src);
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

        void draw_rectangle(framebuffer_t& fb, i32 x, i32 y, i32 w, i32 h, bool fill, color_t src)
        {
            if (fill)
            {
                for (i32 row = y; row < y + h; row++)
                    s_hspan(fb, x, x + w - 1, row, src);
            }
            else
            {
                // Top and bottom edges
                for (i32 px = x; px < x + w; px++)
                {
                    s_put_pixel(fb, px, y, src);
                    s_put_pixel(fb, px, y + h - 1, src);
                }

                // Left and right edges (excluding corner pixels already drawn above)
                for (i32 py = y + 1; py < y + h - 1; py++)
                {
                    s_put_pixel(fb, x, py, src);
                    s_put_pixel(fb, x + w - 1, py, src);
                }
            }
        }

        //  .d8888b.  888 d8b                                 888      8888888b.
        // d88P  Y88b 888 Y8P                                 888      888  "Y88b
        // 888    888 888                                     888      888    888
        // 888        888 888 88888b.  88888b.   .d88b.   .d88888      888    888 888d888 8888b.  888  888  888
        // 888        888 888 888 "88b 888 "88b d8P  Y8b d88" 888      888    888 888P"      "88b 888  888  888
        // 888    888 888 888 888  888 888  888 88888888 888  888      888    888 888    .d888888 888  888  888
        // Y88b  d88P 888 888 888 d88P 888 d88P Y8b.     Y88b 888      888  .d88P 888    888  888 Y88b 888 d88P
        //  "Y8888P"  888 888 88888P"  88888P"   "Y8888   "Y88888      8888888P"  888    "Y888888  "Y8888888P"
        //                    888      888
        //                    888      888
        //                    888      888

        void draw_pixel(framebuffer_t& fb, rect_t const& scissor, i32 x, i32 y, color_t src)
        {
            if (!is_point_in_rect(x, y, scissor))
                return;
            s_put_pixel(fb, x, y, src);
        }

        void draw_line(framebuffer_t& fb, rect_t const& scissor, i32 x0, i32 y0, i32 x1, i32 y1, color_t src)
        {
            if (!s_clip_line(scissor, x0, y0, x1, y1))
                return;

            s_bresenham(fb, x0, y0, x1, y1, src);
        }

        void draw_hline(framebuffer_t& fb, rect_t const& scissor, i32 x0, i32 x1, i32 y, color_t src)
        {
            if (!is_y_in_rect(y, scissor))
                return;

            sort_x(x0, x1);
            if (!is_x_in_rect(x0, scissor))
                return;

            x0 = clip_x_to_rect(x0, scissor);
            x1 = clip_x_to_rect(x1, scissor);

            s_hspan(fb, x0, x1, y, src);
        }

        void draw_vline(framebuffer_t& fb, rect_t const& scissor, i32 x, i32 y0, i32 y1, color_t src)
        {
            if (!is_x_in_rect(x, scissor))
                return;

            sort_y(y0, y1);
            if (!is_y_in_rect(y0, scissor))
                return;

            y0 = clip_y_to_rect(y0, scissor);
            y1 = clip_y_to_rect(y1, scissor);

            s_vspan(fb, x, y0, y1, src);
        }

        void draw_hdline(framebuffer_t& fb, rect_t const& scissor, i32 x0, i32 x1, i32 y, u16 dash1, u16 dash2, color_t src)
        {
            if (!is_y_in_rect(y, scissor))
                return;

            sort_x(x0, x1);
            if (!is_x_in_rect(x0, scissor))
                return;

            x0 = clip_x_to_rect(x0, scissor);
            x1 = clip_x_to_rect(x1, scissor);

            u16 period = dash1 + dash2;
            if (period == 0)
                period = 1;

            i32 i = 0;
            for (i32 x = x0; x <= x1; x++, i++)
            {
                if ((i % period) < dash1)
                    s_put_pixel(fb, x, y, src);
            }
        }

        void draw_vdline(framebuffer_t& fb, rect_t const& scissor, i32 x, i32 y0, i32 y1, u16 dash1, u16 dash2, color_t src)
        {
            if (!is_x_in_rect(x, scissor))
                return;

            sort_y(y0, y1);
            if (!is_y_in_rect(y0, scissor))
                return;

            y0 = clip_y_to_rect(y0, scissor);
            y1 = clip_y_to_rect(y1, scissor);

            u16 period = dash1 + dash2;
            if (period == 0)
                period = 1;

            i32 i = 0;
            for (i32 y = y0; y <= y1; y++, i++)
            {
                if ((i % period) < dash1)
                    s_put_pixel(fb, x, y, src);
            }
        }

        void draw_dline(framebuffer_t& fb, rect_t const& scissor, i32 x0, i32 y0, i32 x1, i32 y1, u16 dash1, u16 dash2, color_t src)
        {
            // Draw the line using Bresenham's algorithm with clipping and dash pattern.
            // Compute the actual start and end points of the line segment after clipping against the scissor rectangle.
            // TODO
        }

        void draw_arc(framebuffer_t& fb, rect_t const& scissor, i32 x, i32 y, i32 radius, f32 start_angle, f32 end_angle, color_t src)
        {
            // Draw the arc using the midpoint circle algorithm with angle constraints.
            // Compute the actual points of the arc after clipping against the scissor rectangle.
            // TODO
        }

        void draw_circle(framebuffer_t& fb, rect_t const& scissor, i32 x, i32 y, i32 radius, bool fill, color_t src)
        {
            // Draw the circle using the midpoint circle algorithm with optional fill.
            // Compute the actual points of the circle after clipping against the scissor rectangle.
            // TODO
        }

        void draw_ellipse(framebuffer_t& fb, rect_t const& scissor, i32 x, i32 y, i32 rx, i32 ry, bool fill, color_t src)
        {
            // Draw the ellipse using the midpoint ellipse algorithm with optional fill.
            // Compute the actual points of the ellipse after clipping against the scissor rectangle.
            // TODO
        }

        void draw_rectangle(framebuffer_t& fb, rect_t const& scissor, i32 x, i32 y, i32 w, i32 h, bool fill, color_t src)
        {
            if (y > scissor.y + scissor.h || y + h < scissor.y || x > scissor.x + scissor.w || x + w < scissor.x)
                return;

            // clip x, y, w, h to scissor
            i32 xs = clip_x_to_rect(x, scissor);
            i32 ys = clip_y_to_rect(y, scissor);
            i32 xe = clip_x_to_rect(x + w, scissor);
            i32 ye = clip_y_to_rect(y + h, scissor);
            draw_rectangle(fb, xs, ys, xe - xs, ye - ys, fill, src);
        }

        void draw_sprite(framebuffer_t& fb, rect_t const& sc, sprite_t* sprite, i32 x, i32 y)
        {
            if (!sprite)
                return;

            u16* fb_pixels = (u16*)fb.pixels;

            const i32 fb_w = (i32)fb.descr.width;
            const i32 fb_h = (i32)fb.descr.height;

            const i32 sprite_x0 = x;
            const i32 sprite_y0 = y;
            const i32 sprite_x1 = x + (i32)sprite->width;
            const i32 sprite_y1 = y + (i32)sprite->height;

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

            // TODO, a sprite might have different pixel formats
            for (i32 j = 0; j < draw_y1 - draw_y0; ++j)
            {
                const u16* src = &((const u16*)sprite->data)[(src_y0 + j) * (i32)sprite->width + src_x0];
                u16*       dst = &fb_pixels[draw_x0 + (draw_y0 + j) * fb_w];
                for (i32 i = 0; i < span_w; ++i, ++src, ++dst)
                {
                    const u16 pixel = *src;
                    *dst            = pixel;
                }
            }
        }

        void draw_sprite(framebuffer_t& fb, rect_t const& sc, sprite_t* sprite, palette_t* palette, i32 x, i32 y)
        {
            if (!sprite)
                return;

            u16* fb_pixels = (u16*)fb.pixels;

            const i32 fb_w = (i32)fb.descr.width;
            const i32 fb_h = (i32)fb.descr.height;

            const i32 sprite_x0 = x;
            const i32 sprite_y0 = y;
            const i32 sprite_x1 = x + (i32)sprite->width;
            const i32 sprite_y1 = y + (i32)sprite->height;

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

            const u16* color_palette = (const u16*)palette->data;

            // TODO, a sprite might have different pixel formats
            for (i32 j = 0; j < draw_y1 - draw_y0; ++j)
            {
                const u8* src = &((const u8*)sprite->data)[(src_y0 + j) * (i32)sprite->width + src_x0];
                u16*      dst = &fb_pixels[draw_x0 + (draw_y0 + j) * fb_w];
                for (i32 i = 0; i < span_w; ++i, ++src, ++dst)
                {
                    const u8 index = *src;
                    *dst           = color_palette[index];
                }
            }
        }

        // ============================================================================
        // CORE GLYPH RENDERER (CLIPPED BOUNDS ASSUMED BY WRAPPER)
        // ============================================================================
        static inline void draw_glyph_sdf_internal(u16* fb, i32 fb_w, i32 out_x, i32 out_y, i32 dst_w, i32 dst_h, i32 clip_x0, i32 clip_y0, i32 clip_x1, i32 clip_y1, const u8* glyph_bitmap, u8 src_w, u16 text_color, float scale)
        {
            // Single hardware FPU float division outside loops
            const float step = 1.0f / scale;

            // Map starting fractional tracking offsets based on top-left clipping constraints
            const float initial_y_offset = (float)(clip_y0 - out_y);
            float y_src                  = initial_y_offset * step;

            // Linear pointer mapping down the hardware framebuffer
            u16* fb_line = &fb[clip_y0 * fb_w];

            for (i32 dy = clip_y0; dy < clip_y1; ++dy)
            {
                i32 row_index = ((i32)y_src) * (i32)src_w;

                const float initial_x_offset = (float)(clip_x0 - out_x);
                float x_src                  = initial_x_offset * step;

                for (i32 dx = clip_x0; dx < clip_x1; ++dx)
                {
                    const i32 x_src_int = (i32)x_src;

                    // Extract the 4-bit distance value from the packed byte stream
                    const i32 pixel_index = row_index + x_src_int;
                    const i32 byte_index  = pixel_index >> 1;
                    const u8  raw_byte    = glyph_bitmap[byte_index];

                    // Branchless bitwise extraction of 4-bit value based on even/odd index
                    const u8 distance = (pixel_index & 1) ? (raw_byte & 0x0F) : (raw_byte >> 4);

                    // Convert distance metric to visibility coverage alpha [0..15]
                    // Threshold matches the Go tool bias mapping: 8 is the vector boundary line
                    // i32 alpha = (i32)distance - 1;
                    // alpha     = (alpha < 0) ? 0 : ((alpha > 15) ? 15 : alpha);

                    const i32 alpha = (distance > 0) ? (distance - 1) : 0;

                    if (alpha > 0)
                    {
                        fb_line[dx] = s_blend_rgb565_a4(fb_line[dx], text_color, (u8)alpha);
                    }

                    x_src += step;
                }

                y_src += step;
                fb_line += fb_w;  // Linear line stepping down the hardware canvas
            }
        }

        // ============================================================================
        // SINGLE GLYPH DRAW CALL
        // ============================================================================
        void draw_glyph_sdf(u16* fb, i32 fb_w, i32 fb_h, i32 pen_x, i32 pen_y, const font_t* font, u8 ascii_char, u16 text_color, float scale)
        {
            // Filter non-ASCII characters out
            if (ascii_char > 127)
                return;

            const u8 glyph_idx = font->m_map[ascii_char];
            if (glyph_idx == 0xFF)
                return;  // Character not supported

            const glyph_t* glyph = &font->m_glyphs[glyph_idx];
            if (glyph->m_width == 0 || glyph->m_height == 0)
                return;

            // Apply layout positions using typographic metrics
            // m_bearing_y is distance from baseline going UP, so we subtract from pen_y
            const i32 out_x = pen_x + (i32)((float)glyph->m_bearing_x * scale);
            const i32 out_y = pen_y - (i32)((float)glyph->m_bearing_y * scale);

            // Compute scaled canvas layout metrics
            const i32 dst_w = (i32)((float)glyph->m_width * scale);
            const i32 dst_h = (i32)((float)glyph->m_height * scale);

            // Calculate edge intersections for standard boundary cropping
            const i32 clip_x0 = (out_x < 0) ? 0 : out_x;
            const i32 clip_y0 = (out_y < 0) ? 0 : out_y;
            const i32 clip_x1 = (out_x + dst_w > fb_w) ? fb_w : out_x + dst_w;
            const i32 clip_y1 = (out_y + dst_h > fb_h) ? fb_h : out_y + dst_h;

            if (clip_x0 >= clip_x1 || clip_y0 >= clip_y1)
                return;

            // Locate the exact starting byte offset of this glyph inside the monolithic SDF binary atlas
            const u8* glyph_bitmap = &font->m_sdf[font->m_offsets[glyph_idx]];

            draw_glyph_sdf_internal(fb, fb_w, out_x, out_y, dst_w, dst_h, clip_x0, clip_y0, clip_x1, clip_y1, glyph_bitmap, glyph->m_width, text_color, scale);
        }

        // ============================================================================
        // ENTIRE STRING/TEXT RENDERER WITH MULTI-LINE SUPPORT
        // ============================================================================
        void draw_text_sdf(u16* fb, i32 fb_w, i32 fb_h, i32 start_x, i32 start_y, const font_t* font, const char* text, u16 text_color, float scale)
        {
            if (!text || scale <= 0.0f)
                return;

            i32 pen_x = start_x;
            i32 pen_y = start_y;

            // Precompute vertical layout stepping metrics
            // Line stride = (Ascent - Descent + LineGap) scaled to the destination canvas
            const i32 total_font_height = (i32)font->m_ascent - (i32)font->m_descent + (i32)font->m_line_gap;
            const i32 line_step_y       = (i32)((float)total_font_height * scale);

            while (*text != '\0')
            {
                const u8 ascii_char = (u8)(*text++);

                // Handle native carriage return/newline string formatting strings blocks
                if (ascii_char == '\n')
                {
                    pen_x = start_x;       // Reset carriage back to margins
                    pen_y += line_step_y;  // Move down to next scanline segment
                    continue;
                }
                if (ascii_char == '\r')
                {
                    continue;  // Skip standard carriage returns explicitly
                }

                if (ascii_char > 127)
                    continue;

                const u8 glyph_idx = font->m_map[ascii_char];
                if (glyph_idx == 0xFF)
                    continue;

                const glyph_t* glyph = &font->m_glyphs[glyph_idx];

                // Draw glyph if it contains printable visual coverage elements
                if (glyph->m_width > 0 && glyph->m_height > 0)
                {
                    const i32 out_x = pen_x + (i32)((float)glyph->m_bearing_x * scale);
                    const i32 out_y = pen_y - (i32)((float)glyph->m_bearing_y * scale);
                    const i32 dst_w = (i32)((float)glyph->m_width * scale);
                    const i32 dst_h = (i32)((float)glyph->m_height * scale);

                    const i32 clip_x0 = (out_x < 0) ? 0 : out_x;
                    const i32 clip_y0 = (out_y < 0) ? 0 : out_y;
                    const i32 clip_x1 = (out_x + dst_w > fb_w) ? fb_w : out_x + dst_w;
                    const i32 clip_y1 = (out_y + dst_h > fb_h) ? fb_h : out_y + dst_h;

                    // Only execute full interpolation loops if the glyph bounding box actually intersects the display view
                    if (clip_x0 < clip_x1 && clip_y0 < clip_y1)
                    {
                        const u8* glyph_bitmap = &font->m_sdf[font->m_offsets[glyph_idx]];
                        draw_glyph_sdf_internal(fb, fb_w, out_x, out_y, dst_w, dst_h, clip_x0, clip_y0, clip_x1, clip_y1, glyph_bitmap, glyph->m_width, text_color, scale);
                    }
                }

                // Always advance the pen layout horizontally, even for spaces or clipped elements
                pen_x += (i32)((float)glyph->m_advance_x * scale);
            }
        }

    }  // namespace ngx2
}  // namespace ncore
