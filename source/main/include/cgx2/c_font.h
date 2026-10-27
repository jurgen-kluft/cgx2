#ifndef __CGX2_C_FONT_H__
#define __CGX2_C_FONT_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
    #pragma once
#endif

#include "cgx2/c_types.h"

namespace ncore
{
    namespace ngx2
    {
        inline font_t* get_font(font_pack_t* pack, u32 font_index)
        {
            if (font_index >= pack->fonts.m_size)
                return nullptr;
            return pack->fonts.item(font_index);
        }

        u16 get_text_y_start(u16 ascent, u16 y, u8 size);
        u16 get_text_y_end(u16 descent, u16 y, u8 size);

    }  // namespace ngx2
}  // namespace ncore

#endif  /// __CGX2_C_FONT_H__
