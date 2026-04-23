#include "ccore/c_target.h"
#include "ccore/c_allocator.h"
#include "ccore/c_math.h"
#include "ccore/c_memory.h"

#include "cgx2/c_gx2.h"
#include "cgx2/c_framebuffer.h"

namespace ncore
{
    namespace ngx2
    {
        // ============================================================================
        // Framebuffer
        // ============================================================================

        inline color_t* rgba8888(void* pixels) { return static_cast<color_t*>(pixels); }  // RGBA8888 pixel buffer
        inline u16*     rgb565(void* pixels) { return static_cast<u16*>(pixels); }        // RGB565 pixel buffer

        void init_framebuffer(framebuffer_t& fb, framedescr_t const& descr, void* pixels)
        {
            fb.descr = descr;
            fb.pixels = pixels;
        }

        void clear_full_framebuffer(framebuffer_t& fb, color_t color)
        {
            if (fb.descr.format == FORMAT_RGBA8888)
            {
                const u32 pixel_count = (u32)fb.descr.width * (u32)fb.descr.height;
                color_t*  pixels      = rgba8888(fb.pixels);
                for (u32 i = 0; i < pixel_count; ++i)
                    pixels[i] = color;
            }
            else if (fb.descr.format == FORMAT_RGB565)
            {
                // Convert RGBA8888 to RGB565 with simple bit-shift truncation (no dithering or error diffusion).
                const u16 c = color_to_rgb565(color);
                const u32 pixel_count = (u32)fb.descr.width * (u32)fb.descr.height;
                u16*      pixels      = rgb565(fb.pixels);
                for (u32 i = 0; i < pixel_count; ++i)
                    pixels[i] = c;
            }
        }

        void quantize_framebuffer(framebuffer_t const& src_framebuffer, framebuffer_t& target_framebuffer)
        {
            // Sanity check
            if (src_framebuffer.pixels == nullptr)
                return;
            // Verify that source and target framebuffers have the same dimensions
            if (src_framebuffer.descr.width != target_framebuffer.descr.width || src_framebuffer.descr.height != target_framebuffer.descr.height)
                return;

            if (src_framebuffer.descr.format == FORMAT_RGBA8888)
            {
                if (target_framebuffer.descr.format == FORMAT_RGB565)
                {
                    const u32 pixel_count = (u32)src_framebuffer.descr.width * (u32)src_framebuffer.descr.height;
                    color_t*  src_pixels  = rgba8888(src_framebuffer.pixels);
                    u16*      dst_pixels  = rgb565(target_framebuffer.pixels);
                    for (u32 i = 0; i < pixel_count; ++i)
                    {
                        dst_pixels[i] = color_to_rgb565(src_pixels[i]);
                    }
                }
            }
        }

        void copy_cell_data(const framebuffer_t& fb, u16 cell_x, u16 cell_y, u8* cell_data, u16& inout_cell_width, u16& inout_cell_height, u32& inout_cell_size_in_bytes)
        {
            const u16 cell_w = inout_cell_width;
            const u16 cell_h = inout_cell_height;

            const u16 x0 = cell_x * cell_w;
            const u16 y0 = cell_y * cell_h;

            // clamp the cell dimensions to the framebuffer bounds
            const u16 cw = ((x0 + cell_w) < fb.descr.width) ? cell_w : (fb.descr.width - x0);
            const u16 ch = ((y0 + cell_h) < fb.descr.height) ? cell_h : (fb.descr.height - y0);

            const u32 bpp               = bytes_per_pixel(fb.descr.format);
            const u32 row_size_in_bytes = bytes_per_row(fb.descr.format, cw);

            u8* dst = cell_data;
            for (u16 y = 0; y < ch; y++)
            {
                const u8* src_row = (const u8*)fb.pixels + ((y0 + y) * fb.descr.width + x0) * bpp;
                g_memcpy(dst, src_row, row_size_in_bytes);
                dst += row_size_in_bytes;
            }

            inout_cell_width         = cw;
            inout_cell_height        = ch;
            inout_cell_size_in_bytes = row_size_in_bytes * ch;
        }

    }  // namespace ngx2
}  // namespace ncore
