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
        struct diff_block_t
        {
            u8  ci;           // Column index of the block (0-based)
            u8  ri;           // Row index of the block (0-based)
            u8  cn;           // Number of contiguous blocks in the same row
            u8  padding;      // Padding for alignment
            u16 w;            // Clipped width of the block (can be less than DIFF_BLOCK_W for edge blocks)
            u16 h;            // Clipped height of the block (can be less than DIFF_BLOCK_H for edge blocks)
            u32 payload_len;  // Length of the data in bytes (w * h * DIFF_BPE)
            u8* payload;      // Pointer to the data for this block
        };

#define DIFF_BLOCK_H 16  // Height of block

        // Initialize a diff_block_t with the provided payload buffer and size.
        // The payload buffer should be large enough to hold the maximum possible
        // block data (width * DIFF_BLOCK_H * DIFF_BPE).
        void diff_block_init(diff_block_t& block, u8* payload_buffer, i32 buffer_size);

        struct diff_engine_t
        {
            u32       width;           // number of elements in a row
            u32       height;          // number of rows
            u32       columns;         // columns = number of blocks in the x direction (width / DIFF_BLOCK_W, rounded up)
            u32       rows;            // rows = number of blocks in the y direction (height / DIFF_BLOCK_H, rounded up)
            u8        bpe;             // bytes per element in the framebuffer (e.g., 4 for RGBA)
            const u8* prev_data;       // pointer to previous data
            const u8* curr_data;       // pointer to current data
            u32       state;           // 0 = uninitialized, 2 = computing, 3 = done
            u32       current_column;  // current block column being processed
            u32       current_row;     // current block row being processed
        };

        void diff_engine_init(diff_engine_t& ctx, u32 width, u32 height, u8 bytesPerElement, const u8* prev_data, const u8* curr_data);
        bool diff_engine_compute(diff_engine_t& ctx, diff_block_t& block);

    }  // namespace ngx2
}  // namespace ncore

#endif  /// __CGX2_DIFF_ENGINE_H__
