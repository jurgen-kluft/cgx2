#ifndef __CGX2_IMAGE_H__
#define __CGX2_IMAGE_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
    #pragma once
#endif

#include "cgx2/c_types.h"

namespace ncore
{
    namespace ngx2
    {
        inline u32 bytes_per_pixel(image_format_t format) { return format & 0xF; }
        inline u32 bytes_per_row(image_format_t format, u32 width) { return width * bytes_per_pixel(format); }

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

#endif  /// __CGX2_IMAGE_H__
