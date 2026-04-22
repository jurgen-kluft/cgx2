#include "ccore/c_target.h"
#include "ccore/c_math.h"
#include "ccore/c_memory.h"

#include "cgx2/c_diff.h"

namespace ncore
{
    namespace ngx2
    {
        static inline u32 s_min_u32(u32 a, u32 b) { return (a < b) ? a : b; }

        static inline bool diff_blocks_equal(const u8* prevData, const u8* currData, u32 numCellsH, u32 numCellsV, u16 cellsPerBlockH, u16 cellsPerBlockV, u16 bytesPerCell, u16 bx, u16 by)
        {
            const u32 blockWidthInCells  = s_min_u32(cellsPerBlockH, numCellsH - (u32)bx * cellsPerBlockH);
            const u32 blockHeightInCells = s_min_u32(cellsPerBlockV, numCellsV - (u32)by * cellsPerBlockV);
            const u32 rowSizeInBytes     = blockWidthInCells * bytesPerCell;
            const u32 src_col_off        = ((u32)bx * cellsPerBlockH) * bytesPerCell;
            u32       src_row_off        = ((u32)by * cellsPerBlockV) * numCellsH * bytesPerCell;
            for (u32 y = 0; y < blockHeightInCells; ++y)
            {
                if (g_memcmp(prevData + src_row_off + src_col_off, currData + src_row_off + src_col_off, rowSizeInBytes) != 0)
                    return false;
                src_row_off += numCellsH * bytesPerCell;
            }
            return true;
        }

        void diff_block_init(diff_block_t& block, u8* payload_buffer, i32 buffer_size)
        {
            block.ci          = 0;
            block.ri          = 0;
            block.cn          = 0;
            block.padding     = 0;
            block.w           = 0;
            block.h           = 0;
            block.payload_len = 0;
            block.payload     = (buffer_size > 0) ? payload_buffer : nullptr;
        }

        void diff_engine_init(diff_engine_t& ctx, u16 widthInCells, u16 heightInCells, u16 cellsPerBlockH, u16 cellsPerBlockV, u8 bytesPerCell, const u8* prevData, const u8* currData)
        {
            ctx.cellCountH      = widthInCells;
            ctx.cellCountV      = heightInCells;
            ctx.cellsPerBlockH  = cellsPerBlockH;
            ctx.cellsPerBlockV  = cellsPerBlockV;
            ctx.blockCountH     = (widthInCells + cellsPerBlockH - 1) / cellsPerBlockH;
            ctx.blockCountV     = (heightInCells + cellsPerBlockV - 1) / cellsPerBlockV;
            ctx.currentBlockH = 0;
            ctx.currentBlockV = 0;
            ctx.prevData       = prevData;
            ctx.currData       = currData;
            ctx.bytesPerCell    = bytesPerCell;
            ctx.state           = 0;  // uninitialized
        }

        // Computes the next diff span that can occur on the current row, starting from the current column.
        // If a span is found, fills in the block info and returns true, otherwise it will continue on to
        // the next row until all rows have been processed, at which point it will return false and the
        // state will be set to 3 (done).
        bool diff_engine_compute(diff_engine_t& ctx, diff_block_t& block)
        {
            if (ctx.state == 3)
                return false;

            if (ctx.state == 0)
            {
                ctx.currentBlockH = 0;
                ctx.currentBlockV = 0;
                ctx.state           = 2;
            }

            const u32 numCellsH = ctx.cellCountH;
            const u32 numCellsV = ctx.cellCountV;

            while (ctx.currentBlockV < ctx.blockCountV)
            {
                const u16 by = ctx.currentBlockV;

                // Find the first changed block on this row.
                while (ctx.currentBlockH < ctx.blockCountH)
                {
                    const u16 bx = ctx.currentBlockH;
                    if (!diff_blocks_equal(ctx.prevData, ctx.currData, numCellsH, numCellsV, ctx.cellsPerBlockH, ctx.cellsPerBlockV, ctx.bytesPerCell, bx, by))
                        break;
                    ++ctx.currentBlockH;
                }

                // No changed span on this row, continue with the next row.
                if (ctx.currentBlockH >= ctx.blockCountH)
                {
                    ctx.currentBlockH = 0;
                    ++ctx.currentBlockV;
                    continue;
                }

                const u16 start_bx = ctx.currentBlockH;
                u16       end_bx   = start_bx;
                while (end_bx < ctx.blockCountH)
                {
                    if (diff_blocks_equal(ctx.prevData, ctx.currData, numCellsH, numCellsV, ctx.cellsPerBlockH, ctx.cellsPerBlockV, ctx.bytesPerCell, end_bx, by))
                        break;
                    ++end_bx;
                }

                const u32 maxSpanCellsW = (u32)(end_bx - start_bx) * ctx.cellsPerBlockH;
                const u32 startCellX    = (u32)start_bx * ctx.cellsPerBlockH;
                const u32 startCellY    = (u32)by * ctx.cellsPerBlockV;

                const u32 spanCellsW  = s_min_u32(maxSpanCellsW, numCellsH - startCellX);
                const u32 spanCellsH  = s_min_u32((u32)ctx.cellsPerBlockV, numCellsV - startCellY);
                const u32 rowBytes    = spanCellsW * ctx.bytesPerCell;
                const u32 payloadSize = rowBytes * spanCellsH;

                block.ci          = (u8)start_bx;
                block.ri          = (u8)by;
                block.cn          = (u8)(end_bx - start_bx);
                block.padding     = 0;
                block.w           = (u16)spanCellsW;
                block.h           = (u16)spanCellsH;
                block.payload_len = payloadSize;

                if (block.payload != nullptr && payloadSize > 0)
                {
                    const u32 src_col_off = startCellX * ctx.bytesPerCell;
                    u32       src_row_off = startCellY * numCellsH * ctx.bytesPerCell;
                    u32       dst_row_off = 0;
                    for (u32 y = 0; y < spanCellsH; ++y)
                    {
                        g_memcpy(block.payload + dst_row_off, ctx.currData + src_row_off + src_col_off, rowBytes);
                        src_row_off += numCellsH * ctx.bytesPerCell;
                        dst_row_off += rowBytes;
                    }
                }

                ctx.currentBlockH = end_bx;
                return true;
            }

            ctx.state           = 3;
            block.ci            = 0;
            block.ri            = 0;
            block.cn            = 0;
            block.padding       = 0;
            block.w             = 0;
            block.h             = 0;
            block.payload_len   = 0;
            ctx.currentBlockH = 0;
            return false;

        }

    }  // namespace ngx2
}  // namespace ncore
