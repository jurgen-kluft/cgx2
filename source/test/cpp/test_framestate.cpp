#include "ccore/c_allocator.h"

#include "cgx2/c_types.h"
#include "cgx2/c_framestate.h"

#include "cunittest/cunittest.h"

using namespace ncore;

UNITTEST_SUITE_BEGIN(gx2)
{
    UNITTEST_FIXTURE(framestate)
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

            ngx2::framedescr_t framedescr = init_framedescr(width, height, ngx2::FORMAT_RGBA8888);
            const u32 bytes_per_frame =ngx2::bytes_per_frame(framedescr);

            u8* prevData = g_allocate_array<u8>(Allocator, bytes_per_frame);
            u8* currData = g_allocate_array<u8>(Allocator, bytes_per_frame);

            // Initialize prevData and currData with some test values
            for (u32 i = 0; i < bytes_per_frame; ++i)
            {
                const u8 b  = i & 0xFF;
                prevData[i] = b;
                currData[i] = b;
            }

            ngx2::framestate_t* framestatePrev = create_framestate(Allocator, framedescr, blockCellCountH, blockCellCountV);
            ngx2::framestate_t* framestateCurr = create_framestate(Allocator, framedescr, blockCellCountH, blockCellCountV);

            update_framestate(framestatePrev, framedescr, prevData);
            update_framestate(framestateCurr, framedescr, currData);

            CHECK_TRUE(compare_framestate(framestatePrev, framestateCurr));

            release_framestate(Allocator, framestatePrev);
            release_framestate(Allocator, framestateCurr);
            g_deallocate(Allocator, prevData);
            g_deallocate(Allocator, currData);
        }

        UNITTEST_TEST(different)
        {
            const u16 blockCellCountH = 16;
            const u16 blockCellCountV = 16;

            u32 width  = 64;
            u32 height = 64;

            ngx2::framedescr_t framedescr = init_framedescr(width, height, ngx2::FORMAT_RGBA8888);
            const u32 bytes_per_frame =ngx2::bytes_per_frame(framedescr);

            u8* prevData = g_allocate_array<u8>(Allocator, bytes_per_frame);
            u8* currData = g_allocate_array<u8>(Allocator, bytes_per_frame);

            // Initialize prevData and currData with some test values
            for (u32 i = 0; i < bytes_per_frame; ++i)
            {
                const u8 b  = i & 0xFF;
                prevData[i] = b;
                currData[i] = b;
            }

            ngx2::framestate_t* framestatePrev = create_framestate(Allocator, framedescr, blockCellCountH, blockCellCountV);
            ngx2::framestate_t* framestateCurr = create_framestate(Allocator, framedescr, blockCellCountH, blockCellCountV);

            update_framestate(framestatePrev, framedescr, prevData);
            update_framestate(framestateCurr, framedescr, currData);

            CHECK_FALSE(compare_framestate(framestatePrev, framestateCurr));

            u32 cell_x = 0;
            u32 cell_y = 0;
            u32 cell_diff_count = 0;
            while (iterate_framestate(framestatePrev, framestateCurr, cell_x, cell_y))
            {
                // Do something with the differing cell coordinates (cell_x, cell_y)
                cell_diff_count++;
            }

            CHECK_TRUE(cell_diff_count > 0);  // We expect at least one differing cell

            release_framestate(Allocator, framestatePrev);
            release_framestate(Allocator, framestateCurr);
            g_deallocate(Allocator, prevData);
            g_deallocate(Allocator, currData);
        }
    }
}
UNITTEST_SUITE_END
