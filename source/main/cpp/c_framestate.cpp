#include "ccore/c_target.h"
#include "ccore/c_allocator.h"
#include "ccore/c_math.h"
#include "ccore/c_memory.h"

#include "cgx2/c_framestate.h"
#include "cgx2/c_types.h"

namespace ncore
{
    namespace ngx2
    {
        inline hash_t s_compute_hash(const u8* data, u32 stride, u32 width_in_bytes, u32 height)
        {
            // compute SHA1 hash of the data and return the lowest 64 bits as hash_t
            return 0;
        }

        framestate_t* create_framestate(alloc_t* allocator, framedescr_t const& fd, u8 cell_size_h, u8 cell_size_v)
        {
            if (!allocator || cell_size_h == 0 || cell_size_v == 0)
                return nullptr;

            const u32 cells_v     = (fd.height + cell_size_v - 1) / cell_size_v;  // Round up to cover partial cells
            const u32 cells_h     = (fd.width + cell_size_h - 1) / cell_size_h;   // Round up to cover partial cells
            const u32 cells_total = cells_h * cells_v;

            const u32 row_hash_array_size  = cells_v * sizeof(hash_t);
            const u32 cell_hash_array_size = cells_total * sizeof(hash_t);

            const u32 alloc_size = sizeof(framestate_t) + row_hash_array_size + cell_hash_array_size;

            framestate_t* state      = (framestate_t*)g_allocate_array_and_clear<u8>(allocator, alloc_size);
            state->m_row_hash_array  = (hash_t*)(state + 1);               // array of row hashes, one per row of cells
            state->m_cell_hash_array = state->m_row_hash_array + cells_v;  // array of cell hashes [row-major order, size = width * height]

            state->m_cell_w     = cell_size_h;
            state->m_cell_h     = cell_size_v;
            state->m_bpp        = bytes_per_pixel(fd.format);  // extract bytes per pixel from format
            state->m_reserved0  = 0;
            state->m_width      = cells_h;  // in cells
            state->m_height     = cells_v;  // in cells
            state->m_frame_hash = 0;
            state->m_reserved1  = 0;

            return state;
        }

        void release_framestate(alloc_t* allocator, framestate_t* state)
        {
            g_deallocate(allocator, state);
        }

        void update_framestate(framestate_t* sb, framedescr_t const& fd, const u8* pixel_data)
        {
            hash_t*   cell_hashes    = sb->m_cell_hash_array;
            const u8* data           = pixel_data;
            const u32 span_size      = fd.width * bytes_per_pixel(fd.format);
            const u8* span           = data;
            const u32 cell_span_size = sb->m_cell_w * (sb->m_bpp / 8);
            for (u32 y = 0; y < fd.height; y += sb->m_cell_h)
            {
                const u8* cell = span;
                for (u32 ss = 0; ss < span_size; ss += cell_span_size)
                {
                    // clamp the 'cell offset' and 'cell height' to the framebuffer size
                    const u32 cw   = (ss + cell_span_size < span_size) ? cell_span_size : (span_size - ss);
                    const u32 ch   = (y + sb->m_cell_h < fd.height) ? sb->m_cell_h : (fd.height - y);
                    hash_t    hash = s_compute_hash(cell, span_size, cw, ch);
                    *cell_hashes++ = hash;
                    cell += cell_span_size;
                }
                span += sb->m_cell_h * span_size;
            }

            // compute the row hashes
            hash_t* row_hashes = sb->m_row_hash_array;
            cell_hashes        = sb->m_cell_hash_array;
            for (u32 row = 0; row < sb->m_height; row++)
            {
                *row_hashes++ = s_compute_hash((const u8*)cell_hashes, sb->m_width * sizeof(hash_t), sb->m_width * sizeof(hash_t), 1);
                cell_hashes += sb->m_width;
            }

            // compute the frame hash
            sb->m_frame_hash = s_compute_hash((const u8*)sb->m_row_hash_array, sb->m_height * sizeof(hash_t), sb->m_height * sizeof(hash_t), 1);
            
        }

        bool compare_framestate(const framestate_t* prevState, const framestate_t* currState)
        {
            // Quick check: if the frame hashes are different, we know for sure the frames are different
            if (prevState->m_frame_hash != currState->m_frame_hash)
                return false;

            // If the frame hashes are the same, we still need to check the row and cell hashes to be sure
            for (u32 i = 0; i < prevState->m_height; i++)
            {
                if (prevState->m_row_hash_array[i] != currState->m_row_hash_array[i])
                    return false;
            }

            const u32 num_pixels = prevState->m_width * prevState->m_height;
            for (u32 i = 0; i < num_pixels; i++)
            {
                if (prevState->m_cell_hash_array[i] != currState->m_cell_hash_array[i])
                    return false;
            }

            return true;  // All hashes match, frames are considered identical
        }

        bool iterate_framestate(const framestate_t* prevState, const framestate_t* currState, u32& cell_x, u32& cell_y)
        {
            const u32 total_cells = prevState->m_width * prevState->m_height;
            for (u32 i = cell_y * prevState->m_width + cell_x; i < total_cells; i++)
            {
                if (prevState->m_cell_hash_array[i] != currState->m_cell_hash_array[i])
                {
                    cell_y = i / prevState->m_width;
                    cell_x = i - (cell_y * prevState->m_width);
                    return true;
                }
            }
            return false;  // No more differences
        }

    }  // namespace ngx2
}  // namespace ncore
