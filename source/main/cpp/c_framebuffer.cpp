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
        
        inline color_t* rgba8888(framebuffer_t* fb) { return static_cast<color_t*>(fb->pixels); }  // RGBA8888 pixel buffer
        inline u16*     rgb565(framebuffer_t* fb) { return static_cast<u16*>(fb->pixels); }        // RGB565 pixel buffer

        framebuffer_t* new_framebuffer(alloc_t* alloc, u16 width, u16 height, image_format_t format)
        {
            framebuffer_t* fb = (framebuffer_t*)alloc->allocate(sizeof(framebuffer_t));
            if (fb)
            {
                fb->width  = width;
                fb->height = height;
                fb->format = format;

                const u32 pixel_count = (u32)width * (u32)height;
                switch (format)
                {
                    case FORMAT_RGBA8888: fb->pixels = g_allocate_array<color_t>(alloc, pixel_count); break;
                    case FORMAT_RGB565: fb->pixels = g_allocate_array<u16>(alloc, pixel_count); break;
                    default: alloc->deallocate(fb); return nullptr;
                }
            }
            return fb;
        }

        void release_framebuffer(framebuffer_t* framebuffer, alloc_t* allocator)
        {
            if (framebuffer)
            {
                g_deallocate_array(allocator, framebuffer->pixels);
                allocator->deallocate(framebuffer);
            }
        }

        void clear_full_framebuffer(context_t* ctx, color_t color)
        {
            framebuffer_t* fb = ctx->framebuffer;
            if (fb->format == FORMAT_RGBA8888)
            {
                const u32 pixel_count = (u32)fb->width * (u32)fb->height;
                color_t*  pixels      = rgba8888(fb);
                for (u32 i = 0; i < pixel_count; ++i)
                    pixels[i] = color;
            }
            else if (fb->format == FORMAT_RGB565)
            {
                // Convert RGBA8888 to RGB565 with simple bit-shift truncation (no dithering or error diffusion).
                const u16 r5 = color.r >> 3;
                const u16 g6 = color.g >> 2;
                const u16 b5 = color.b >> 3;
                const u16 c  = (r5 << 11) | (g6 << 5) | b5;

                const u32 pixel_count = (u32)fb->width * (u32)fb->height;
                u16*      pixels      = rgb565(fb);
                for (u32 i = 0; i < pixel_count; ++i)
                    pixels[i] = c;
            }
        }

        void quantize_framebuffer(context_t* ctx, framebuffer_t* target_framebuffer)
        {
            // Sanity check
            if (ctx->framebuffer == nullptr || target_framebuffer == nullptr)
                return;
            // Verify that source and target framebuffers have the same dimensions
            if (ctx->framebuffer->width != target_framebuffer->width || ctx->framebuffer->height != target_framebuffer->height)
                return;

            framebuffer_t* src = ctx->framebuffer;
            if (src->format == FORMAT_RGBA8888)
            {
                if (target_framebuffer->format == FORMAT_RGB565)
                {
                    const u32 pixel_count = (u32)src->width * (u32)src->height;
                    color_t*  src_pixels  = rgba8888(src);
                    u16*      dst_pixels  = rgb565(target_framebuffer);
                    for (u32 i = 0; i < pixel_count; ++i)
                    {
                        const color_t& c  = src_pixels[i];
                        const u16      r5 = c.r >> 3;
                        const u16      g6 = c.g >> 2;
                        const u16      b5 = c.b >> 3;
                        dst_pixels[i]     = (r5 << 11) | (g6 << 5) | b5;
                    }
                }
            }
        }

        void copy_cell_data(const framebuffer_t* fb, u16 cell_x, u16 cell_y, u8* cell_data, u16& inout_cell_width, u16& inout_cell_height, u32& inout_cell_size_in_bytes)
        {
            const u16 cell_w = inout_cell_width;
            const u16 cell_h = inout_cell_height;

            const u16 x0 = cell_x * cell_w;
            const u16 y0 = cell_y * cell_h;

            // clamp the cell dimensions to the framebuffer bounds
            const u16 cw = ((x0 + cell_w) < fb->width) ? cell_w : (fb->width - x0);
            const u16 ch = ((y0 + cell_h) < fb->height) ? cell_h : (fb->height - y0);

            const u32 bytes_per_pixel   = (fb->format == FORMAT_RGBA8888) ? 4 : 2;
            const u32 row_size_in_bytes = cw * bytes_per_pixel;

            u8* dst = cell_data;
            for (u16 y = 0; y < ch; y++)
            {
                const u8* src_row = (const u8*)fb->pixels + ((y0 + y) * fb->width + x0) * bytes_per_pixel;
                g_memcpy(dst, src_row, row_size_in_bytes);
                dst += row_size_in_bytes;
            }

            inout_cell_width         = cw;
            inout_cell_height        = ch;
            inout_cell_size_in_bytes = row_size_in_bytes * ch;
        }

    }  // namespace ngx2
}  // namespace ncore
