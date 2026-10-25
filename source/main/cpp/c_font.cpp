#include "ccore/c_target.h"
#include "ccore/c_math.h"
#include "ccore/c_memory.h"

#include "cgx2/c_types.h"

#include <cmath>

namespace ncore
{
    namespace ngx2
    {
        // Note: 8.8 fixed-point calculations
        u16 get_text_y_start(const font_t* font, u16 y, u8 size)
        {
            const u32 scale = (static_cast<u32>(size) << 16) / font->m_size;
            const u32 ascent = static_cast<u32>((font->m_ascent * scale) >> 8) + 0x0080;  // rounding
            return y - static_cast<u16>(ascent >> 8);
        }

        u16 get_text_y_end(const font_t* font, u16 y, u8 size)
        {
            const u32 scale = (static_cast<u32>(size) << 16) / font->m_size;
            const u32 descent = static_cast<u32>((font->m_descent * scale) >> 8) + 0x0080;  // rounding
            return y + static_cast<u16>(descent >> 8);
        }

    }  // namespace ngx2
}  // namespace ncore
