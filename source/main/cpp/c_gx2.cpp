#include "ccore/c_target.h"
#include "cgx2/c_gx2.h"

#include <cmath>

namespace ncore
{
    namespace ngx2
    {
        // ============================================================================
        // Framebuffer
        // ============================================================================

        struct framebuffer_t
        {
            u32  width;   // width in pixels
            u32  height;  // height in pixels
            u32* pixels;  // RGBA8888 pixel buffer
        };

        // ============================================================================
        // Blend + Draw State
        // ============================================================================

        struct draw_state_t
        {
            color_t       color;      // current drawing color
            blend_state_t blend;      // current blend state
            rect_t        scissor;    // active scissor rect
            u8            fill;       // 0 or 1
            u16           thickness;  // stroke width in pixels
            f32           rotation;   // degrees
            f32           scale_x;    // horizontal scale factor
            f32           scale_y;    // vertical scale factor
            sprite_t*     sprite;     // currently bound sprite (nullable)
            font_t*       font;       // currently bound font (nullable)
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
            u32        width;
            u32        height;
            const u32* pixels;
        };

        struct sprite_context_t
        {
            sprite_t* sprites;
            u32       count;
            u32       capacity;
        };

        // ============================================================================
        // Font System
        // ============================================================================

        struct glyph_t
        {
            i16       advance_x;  // how much to move the pen horizontally to the next character after drawing this one
            i16       bearing_x;  // horizontal distance from the pen position to the left edge of the glyph bitmap
            i16       bearing_y;  // vertical distance from the pen position to the top edge of the glyph bitmap (can be negative)
            u16       width;      // width of the glyph bitmap in pixels
            u16       height;     // height of the glyph bitmap in pixels
            const u8* bitmap;     // alpha or coverage bitmap
        };

        struct font_t
        {
            glyph_t* glyphs;    // array of glyphs, indexed by glyph index (not ASCII code)
            u8       map[256];  // maps ASCII character codes to glyph indices in the glyphs array, or 0xFF if the character is not supported
            i16      ascent;    // distance from baseline to top of font
            i16      descent;   // distance from baseline to bottom of font (negative value)
            i16      line_gap;  // distance from bottom of one line to top of next line (can be negative)
        };

        struct font_context_t
        {
            font_t* fonts;
            u32     font_count;
            u32     font_capacity;
        };

        // ============================================================================
        // Geometry & Utility Functions

        enum blend_function_e
        {
            BLENDFN_ZERO,
            BLENDFN_ONE,
            BLENDFN_SRC_COLOR,
            BLENDFN_ONE_MINUS_SRC_COLOR,
            BLENDFN_DST_COLOR,
            BLENDFN_ONE_MINUS_DST_COLOR,
            BLENDFN_SRC_ALPHA,
            BLENDFN_ONE_MINUS_SRC_ALPHA,
            BLENDFN_DST_ALPHA,
            BLENDFN_ONE_MINUS_DST_ALPHA,
            BLENDFN_CONSTANT_COLOR,
            BLENDFN_ONE_MINUS_CONSTANT_COLOR,
            BLENDFN_CONSTANT_ALPHA,
            BLENDFN_ONE_MINUS_CONSTANT_ALPHA,
        };

        enum blend_equation_e
        {
            BLENDEQ_FUNC_ADD,
            BLENDEQ_FUNC_SUBTRACT,
            BLENDEQ_FUNC_REVERSE_SUBTRACT,
            BLENDEQ_MIN,
            BLENDEQ_MAX,
        };

        static bool s_is_in_ellipse(i32 x, i32 y, i32 x0, i32 y0, i32 a, i32 b, float angle)
        {
            // Check if a point is in an ellipse with center (x0, y0), semi-major axis a, semi-minor axis b, and angle of rotation angle
            // https://en.wikipedia.org/wiki/Ellipse#Equation_of_a_general_ellipse

            // Rotate the point
            i32 k = cos(angle) * (x - x0) + sin(angle) * (y - y0);
            i32 l = sin(angle) * (x - x0) - cos(angle) * (y - y0);

            // Check if the point is in the ellipse
            return (k * b) * (k * b) + (l * a) * (l * a) <= a * a * b * b;
        }

        static void s_draw_circle(u32* pixels, u32 width, u32 height, i32 cx, i32 cy, i32 radius, u32 color)
        {
            // use the symmetry of the circle and only checking one octant
            i32 x   = radius;
            i32 y   = 0;
            i32 err = 0;

            i32 cxpx = cx + x;
            i32 cxmx = cx - x;
            i32 cxpy = cy + y;
            i32 cxmy = cy - y;

            i32 cypx = cy + x;
            i32 cymx = cy - x;
            i32 cypy = cy + y;
            i32 cymy = cy - y;

            while (x >= y)
            {
                if (0 < cxpx && cxpx < (i32)width)
                {
                    if (0 < cxpy && cxpy < (i32)height)
                        pixels[cxpx + cxpy * width] = color;
                    if (0 < cxmy && cxmy < (i32)height)
                        pixels[cxpx + cxmy * width] = color;
                }
                if (0 < cxmx && cxmx < (i32)width)
                {
                    if (0 < cxpy && cxpy < (i32)height)
                        pixels[cxmx + cxpy * width] = color;
                    if (0 < cxmy && cxmy < (i32)height)
                        pixels[cxmx + cxmy * width] = color;
                }
                if (0 < cxpy && cxpy < (i32)height)
                {
                    if (0 < cypx && cypx < (i32)width)
                        pixels[cypx + cy * width] = color;
                    if (0 < cymx && cymx < (i32)width)
                        pixels[cymx + cy * width] = color;
                }
                if (0 < cxmy && cxmy < (i32)height)
                {
                    if (0 < cypx && cypx < (i32)width)
                        pixels[cypx + cy * width] = color;
                    if (0 < cymx && cymx < (i32)width)
                        pixels[cymx + cy * width] = color;
                }

                if (err <= 0)
                {
                    err += 2 * y + 1;
                    y++;
                    cxpy = cy + y;
                    cxmy = cy - y;
                    cypy = cy + x;
                    cymy = cy - x;
                }
                else
                {
                    err -= 2 * x - 1;
                    x--;
                    cxpx = cx + x;
                    cxmx = cx - x;
                    cypx = cy + x;
                    cymx = cy - x;
                }
            }
        }

        // Bresenham's line algorithm : https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
        static void s_draw_line(u32* pixels, u32 width, u32 height, i32 x0, i32 y0, i32 x1, i32 y1, u32 color)
        {
            i32 dx  = abs(x1 - x0);
            i32 sx  = x0 < x1 ? 1 : -1;
            i32 dy  = -abs(y1 - y0);
            i32 sy  = y0 < y1 ? 1 : -1;
            i32 err = dx + dy;
            i32 e2;

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

    }  // namespace ngx2
}  // namespace ncore
