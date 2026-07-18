#ifndef __CGX2_FRAME_BUFFER_H__
#define __CGX2_FRAME_BUFFER_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
    #pragma once
#endif

#include "cgx2/c_types.h"

namespace ncore
{
    class alloc_t;

    namespace ngx2
    {
        // ============================================================================
        // Framebuffer
        // ============================================================================

        void init_framebuffer(framebuffer_t& fb, image_descr_t const& descr, void* pixels);
        void clear_full_framebuffer(framebuffer_t& fb, color_t color);

    }  // namespace ngx2
}  // namespace ncore

#endif  /// __CGX2_FRAME_BUFFER_H__
