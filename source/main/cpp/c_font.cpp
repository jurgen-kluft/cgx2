#include "ccore/c_target.h"
#include "ccore/c_math.h"
#include "ccore/c_memory.h"

#include "cgx2/c_types.h"

#include <cmath>

namespace ncore
{
    namespace ngx2
    {
        // Note: ascent/descent/line_gap are all in 8.8 fixed-point format and relative to size == 32
        const u32 cReferenceSizeFixed = 32u << 8;
        u16       get_text_y_start(u16 ascent, u16 y, u8 size)
        {
            const u32 size_fixed   = static_cast<u32>(size) << 8;
            const u32 scale        = (size_fixed << 16) / cReferenceSizeFixed;
            const u32 ascent_fixed = (ascent * scale + 0x8000) >> 16;
            return y - static_cast<u16>(ascent_fixed);
        }

        u16 get_text_y_end(u16 descent, u16 y, u8 size)
        {
            const u32 size_fixed    = static_cast<u32>(size) << 8;
            const u32 scale         = (size_fixed << 16) / cReferenceSizeFixed;
            const u32 descent_fixed = (descent * scale + 0x8000) >> 16;
            return y + static_cast<u16>(descent_fixed);
        }

    }  // namespace ngx2
}  // namespace ncore
