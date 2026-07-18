#include "ccore/c_target.h"
#include "ccore/c_allocator.h"
#include "ccore/c_math.h"
#include "ccore/c_memory.h"

#include "cgx2/c_framebuffer.h"
#include "cgx2/c_types.h"

namespace ncore
{
    namespace ngx2
    {
        // ============================================================================
        // Framebuffer
        // ============================================================================

        inline color_t* rgba8888(void* pixels) { return static_cast<color_t*>(pixels); }  // RGBA8888 pixel buffer
        inline u16*     rgb565(void* pixels) { return static_cast<u16*>(pixels); }        // RGB565 pixel buffer

        void init_framebuffer(framebuffer_t& fb, image_descr_t const& descr, void* pixels)
        {
            fb.descr = descr;
            fb.pixels = pixels;
        }

        void clear_full_framebuffer(framebuffer_t& fb, color_t color)
        {
            if (fb.descr.format == IMAGE_FORMAT_RGB565)
            {
                // Convert RGBA8888 to RGB565 with simple bit-shift truncation (no dithering or error diffusion).
                const u16 c = color;
                const u32 pixel_count = (u32)fb.descr.width * (u32)fb.descr.height;
                u16*      pixels      = rgb565(fb.pixels);
                for (u32 i = 0; i < pixel_count; ++i)
                    pixels[i] = c;
            }
        }

    }  // namespace ngx2
}  // namespace ncore
