#ifndef __CGX2_PALETTE_H__
#define __CGX2_PALETTE_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
    #pragma once
#endif

#include "cgx2/c_types.h"

namespace ncore
{
    namespace ngx2
    {
        inline const palette_t* get_palette(const palette_pack_t* palette_pack, u32 index)
        {
            if (index < palette_pack->m_palettes.m_size)
                return palette_pack->m_palettes.item(index);
            return nullptr;
        }

        const palette_t* get_palette(const palette_pack_t* palette_pack, sprite_t const* sprite)
        {
            switch (sprite->m_pixel_format)
            {
                case FMT_PIXEL_I8: return get_palette(palette_pack, sprite->m_palette_index);
            }
            return nullptr;
        }

    }  // namespace ngx2
}  // namespace ncore

#endif  /// __CGX2_PALETTE_H__
