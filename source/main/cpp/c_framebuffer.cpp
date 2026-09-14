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
            fb.width  = descr.width;
            fb.height = descr.height;
            fb.pixels = rgb565(pixels);
        }

        void clear_full_framebuffer(framebuffer_t& fb, color_t color)
        {
            if (fb.pixels && fb.width && fb.height) // Assuming RGB565
            {
                const u32 pixel_count = (u32)fb.width * (u32)fb.height;
                u16*      pixels      = rgb565(fb.pixels);
                for (u32 i = 0; i < pixel_count; ++i)
                    pixels[i] = color;
            }
        }

    }  // namespace ngx2
}  // namespace ncore
