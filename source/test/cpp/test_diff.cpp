#include "ccore/c_allocator.h"

#include "cgx2/c_diff.h"

#include "cunittest/cunittest.h"

using namespace ncore;

UNITTEST_SUITE_BEGIN(gx2)
{
    UNITTEST_FIXTURE(diff)
    {
        UNITTEST_FIXTURE_SETUP() {}
        UNITTEST_FIXTURE_TEARDOWN() {}

        UNITTEST_ALLOCATOR;

        UNITTEST_TEST(identical)
        {
            const u16 blockCellCountH = 16;
            const u16 blockCellCountV = 16;

            u32 width  = 64;
            u32 height = 64;
            u8  bpe    = 1;

            u8* prevData = g_allocate_array<u8>(Allocator, width * height * bpe);
            u8* currData = g_allocate_array<u8>(Allocator, width * height * bpe);

            // Initialize prevData and currData with some test values
            for (u32 i = 0; i < width * height * bpe; ++i)
            {
                const u8 b  = i & 0xFF;
                prevData[i] = b;
                currData[i] = b;
            }

            ngx2::diff_engine_t engine;
            ngx2::diff_engine_init(engine, width, height, blockCellCountH, blockCellCountV, bpe, prevData, currData);

            // Buffer for the maximum number of blocks that can be contiguous in a single row
            // (worst case is every block is different, so we need blockCountH blocks * blockCountV rows)
            u32 payloadBufferSize = engine.blockCountH * blockCellCountH * blockCellCountV * bpe;
            u8* payload_buffer = g_allocate_array<u8>(Allocator, payloadBufferSize);
            ngx2::diff_block_t block;
            ngx2::diff_block_init(block, payload_buffer, 0);  // No payload buffer since we expect no blocks to be generated

            while (ngx2::diff_engine_compute(engine, block))
            {
                // Since the data is identical, we expect no blocks to be generated
                ASSERT(block.payload_len == 0);
            }

            g_deallocate(Allocator, payload_buffer);
            g_deallocate(Allocator, prevData);
            g_deallocate(Allocator, currData);
        }

        UNITTEST_TEST(different)
        {
            const u16 blockCellCountH = 16;
            const u16 blockCellCountV = 16;

            u32 width  = 64;
            u32 height = 64;
            u8  bpe    = 1;

            u8* prevData = g_allocate_array<u8>(Allocator, width * height * bpe);
            u8* currData = g_allocate_array<u8>(Allocator, width * height * bpe);

            // Initialize prevData and currData with some test values
            for (u32 i = 0; i < width * height * bpe; ++i)
            {
                const u8 b  = i & 0xFF;
                prevData[i] = b;
                currData[i] = b;
            }

            // Introduce some differences in currData
            for (u32 i = 0; i < width * height * bpe; i += 100)
            {
                currData[i] = (currData[i] + 1);
            }

            ngx2::diff_engine_t engine;
            ngx2::diff_engine_init(engine, width, height, blockCellCountH, blockCellCountV, bpe, prevData, currData);

            // Buffer for the maximum number of blocks that can be contiguous in a single row
            // (worst case is every block is different, so we need blockCountH blocks * blockCountV rows)
            u32 payloadBufferSize = engine.blockCountH * blockCellCountH * blockCellCountV * bpe;
            u8* payload_buffer = g_allocate_array<u8>(Allocator, payloadBufferSize);
            ngx2::diff_block_t block;
            ngx2::diff_block_init(block, payload_buffer, payloadBufferSize);

            while (ngx2::diff_engine_compute(engine, block))
            {
                // Since the data is different, we expect blocks to be generated
                ASSERT(block.payload_len > 0);
                ASSERT(block.payload != nullptr);
                // We can also check the contents of the block if needed
            }

            g_deallocate(Allocator, payload_buffer);
            g_deallocate(Allocator, prevData);
            g_deallocate(Allocator, currData);
        }
    }
}
UNITTEST_SUITE_END
