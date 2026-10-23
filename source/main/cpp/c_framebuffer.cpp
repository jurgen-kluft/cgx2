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

        void init_framebuffer(framebuffer_t& fb, image_descr_t const& descr, void* pixels)
        {
            fb.m_width  = descr.m_width;
            fb.m_height = descr.m_height;
            fb.m_pixels = (u16*)(pixels);
        }

        void clear_full_framebuffer(framebuffer_t& fb, color_t color)
        {
            if (fb.m_pixels && fb.m_width && fb.m_height) // Assuming RGB565
            {
                const u32 pixel_count = (u32)fb.m_width * (u32)fb.m_height;
                u16*      pixels      = fb.m_pixels;
                for (u32 i = 0; i < pixel_count; ++i)
                    pixels[i] = color;
            }
        }

    }  // namespace ngx2
}  // namespace ncore
