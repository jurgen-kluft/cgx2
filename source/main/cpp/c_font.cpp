#include "ccore/c_target.h"
#include "ccore/c_math.h"
#include "ccore/c_memory.h"

#include "cgx2/c_types.h"

#include <cmath>

namespace ncore
{
    namespace ngx2
    {
        // Note: ascent/descent/line_gap are values relative to the size of the font, so we need to scale
        //       them accordingly. Here we use fixed point arithmetic for scaling.
        u16 get_text_y_start(u8 font_size, u8 ascent, u16 y, u8 requested_size)
        {
            const u32 scale           = ((u32)requested_size << 8) / font_size;
            const u32 ascent_fixed    = (scale * ascent + 0x80) >> 8;
            return y - static_cast<u16>(ascent_fixed);
        }

        u16 get_text_y_end(u8 font_size, u8 descent, u16 y, u8 requested_size)
        {
            const u32 scale           = ((u32)requested_size << 8) / font_size;
            const u32 descent_fixed   = (scale * descent + 0x80) >> 8;
            return y + static_cast<u16>(descent_fixed);
        }

    }  // namespace ngx2
}  // namespace ncore
