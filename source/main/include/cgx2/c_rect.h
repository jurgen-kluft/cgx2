#ifndef __CGX2_RECT_H__
#define __CGX2_RECT_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
    #pragma once
#endif

#include "cgx2/c_types.h"

namespace ncore
{
    namespace ngx2
    {
        inline bool is_y_in_rect(i32 y, rect_t const& r) { return (y >= r.y && y < r.y + r.h); }
        inline bool is_x_in_rect(i32 x, rect_t const& r) { return (x >= r.x && x < r.x + r.w); }
        inline bool is_point_in_rect(i32 px, i32 py, rect_t const& r) { return is_x_in_rect(px, r) && is_y_in_rect(py, r); }
        inline i32  clip_y_to_rect(i32 y, rect_t const& r) { return (y < r.y) ? r.y : ((y >= r.y + r.h) ? (r.y + r.h - 1) : y); }
        inline i32  clip_x_to_rect(i32 x, rect_t const& r) { return (x < r.x) ? r.x : ((x >= r.x + r.w) ? (r.x + r.w - 1) : x); }

        inline void sort_x(i32& x0, i32& x1)
        {
            if (x0 > x1)
            {
                i32 t = x0;
                x0    = x1;
                x1    = t;
            }
        }
        inline void sort_y(i32& y0, i32& y1)
        {
            if (y0 > y1)
            {
                i32 t = y0;
                y0    = y1;
                y1    = t;
            }
        }

        // ============================================================================
        inline u32 bytes_per_pixel(image_format_t format) { return format & 0xF; }
        inline u32 bytes_per_row(image_format_t format, u32 width) { return width * bytes_per_pixel(format); }

        // ============================================================================
        inline image_descr_t init_image_descr(u16 width, u16 height, image_format_t format)
        {
            image_descr_t descr;
            descr.m_width    = width;
            descr.m_height   = height;
            descr.m_reserved = 0;
            descr.m_format   = format;
            return descr;
        }

        inline u32 bytes_per_image(image_descr_t const& descr) { return descr.m_height * descr.m_width * bytes_per_pixel(descr.m_format); }

    }  // namespace ngx2
}  // namespace ncore

#endif  /// __CGX2_RECT_H__
