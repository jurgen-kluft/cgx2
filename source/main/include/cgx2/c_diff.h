#ifndef __CGX2_DIFF_ENGINE_H__
#define __CGX2_DIFF_ENGINE_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
    #pragma once
#endif

namespace ncore
{
    namespace ngx2
    {
        typedef u32 hash_t;

        struct diff_state_t
        {
            u8      m_cell_w;           // cell width in pixels
            u8      m_cell_h;           // cell height in pixels
            u8      m_bpp;              // bits per pixel
            u8      m_reserved0;        // reserved for future use
            u16     m_width;            // in cells
            u16     m_height;           // in cells
            hash_t  m_frame_hash;       // hash of the entire frame
            u32     m_reserved1;        // reserved for future use
            hash_t* m_row_hash_array;   // array of row hashes, one per row of cells
            hash_t* m_cell_hash_array;  // array of cell hashes [row-major order, size = width * height]
        };

        struct framebuffer_t;

        // Create a diff_state_t for the provided frame buffer.
        diff_state_t* create_diff_state(alloc_t* allocator, framebuffer_t* fb, u8 cell_size_h, u8 cell_size_v);
        void          update_diff_state(diff_state_t* diffState, framebuffer_t* fb);
        bool          compare_diff_state(const diff_state_t* prevState, const diff_state_t* currState);

        // Iterate through the differences between two diff states, start with cell_x = 0, cell_y = 0.
        // The function will update cell_x and cell_y to the next different cell coordinates.
        // Returns true if there are more differences to iterate, false when done.
        bool iterate_diff(const diff_state_t* prevState, const diff_state_t* currState, u32& cell_x, u32& cell_y);

    }  // namespace ngx2
}  // namespace ncore

#endif  /// __CGX2_DIFF_ENGINE_H__
