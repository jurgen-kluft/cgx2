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

        struct framebuffer_t
        {
            framedescr_t descr;   // framebuffer description (width, height, format)
            void*        pixels;  // pixel data
        };

        void init_framebuffer(framebuffer_t& fb, framedescr_t const& descr, void* pixels);
        void clear_full_framebuffer(framebuffer_t& fb, color_t color);
        void quantize_framebuffer(framebuffer_t const& src_framebuffer, framebuffer_t& target_framebuffer);

        // Example:
        //  u16 cell_width = 16; cell_height = 16;
        //  u32 cell_size_in_bytes = cell_width * cell_height * bytes_per_pixel(fb->format);
        //  u8 cell_data[cell_size_in_bytes];
        //  copy_cell_data(fb, cell_x, cell_y, cell_data, cell_width, cell_height, cell_size_in_bytes);
        //  Note: cell_width, cell_height, and cell_size_in_bytes are input/output parameters.
        //        The function will update them to the actual size of the copied cell data (clamped to framebuffer bounds).
        void copy_cell_data(const framebuffer_t& fb, u16 cell_x, u16 cell_y, u8* cell_data, u16& inout_cell_width, u16& inout_cell_height, u32& inout_cell_size_in_bytes);

    }  // namespace ngx2
}  // namespace ncore

#endif  /// __CGX2_FRAME_BUFFER_H__
