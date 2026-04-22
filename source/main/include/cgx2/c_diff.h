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

        // Initialize a diff_block_t with the provided payload buffer and size.
        // The payload buffer should be large enough to hold the maximum possible
        // block data (width * DIFF_BLOCK_H * DIFF_BPE).
        void diff_block_init(diff_block_t& block, u8* payload_buffer, i32 buffer_size);

        struct diff_engine_t
        {
            u16       cellCountH;      // number of cells on the horizontal axis
            u16       cellCountV;      // number of cells on the vertical axis
            u16       cellsPerBlockH;  // width of a block in cells (e.g. 16)
            u16       cellsPerBlockV;  // height of a block in cells (e.g. 16)
            u16       blockCountH;     // number of blocks in the horizontal direction (cellCountH / (cellsPerBlockH*bytesPerCell), rounded up)
            u16       blockCountV;     // number of blocks in the vertical direction (cellCountV / cellsPerBlockV, rounded up)
            u16       currentBlockH;   // current block on the horizontal axis being processed
            u16       currentBlockV;   // current block on the vertical axis being processed
            const u8* prevData;        // pointer to previous data
            const u8* currData;        // pointer to current data
            u16       bytesPerCell;    // bytes per cell in the framebuffer (e.g. 4 for RGBA)
            u16       state;           // 0 = uninitialized, 2 = computing, 3 = done
        };

        void diff_engine_init(diff_engine_t& ctx, u16 widthInCells, u16 heightInCells, u16 cellsPerBlockH, u16 cellsPerBlockV, u8 bytesPerCell, const u8* prevData, const u8* currData);
        bool diff_engine_compute(diff_engine_t& ctx, diff_block_t& block);

    }  // namespace ngx2
}  // namespace ncore

#endif  /// __CGX2_DIFF_ENGINE_H__
