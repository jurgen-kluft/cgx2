#ifndef __CGX2_COLOR_H__
#define __CGX2_COLOR_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
    #pragma once
#endif

namespace ncore
{
    class alloc_t;

    namespace ngx2
    {
        typedef u16 color_t;

        inline u16 rgb888_to_rgb565(u32 color)
        {
            const u32 r = (color >> 16) & 0xFF;
            const u32 g = (color >> 8) & 0xFF;
            const u32 b = color & 0xFF;
            return (u16)(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
        }

    }  // namespace ngx2
}  // namespace ncore

#endif  /// __CGX2_COLOR_H__
