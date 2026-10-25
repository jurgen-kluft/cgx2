#ifndef __CGX2_SPRITE_H__
#define __CGX2_SPRITE_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
    #pragma once
#endif

#include "cgx2/c_types.h"

namespace ncore
{
    namespace ngx2
    {
        inline sprite_t* get_sprite(sprite_pack_t* sprite_pack, u32 index)
        {
            if (index < sprite_pack->sprites.m_size)
                return sprite_pack->sprites.item(index);
            return nullptr;
        }

        inline rect_t get_bbox(sprite_t* sprite)
        {
            rect_t bbox;
            bbox.x = 0;
            bbox.y = 0;
            bbox.w = sprite->m_width;
            bbox.h = sprite->m_height;
            return bbox;
        }

    }  // namespace ngx2
}  // namespace ncore

#endif  /// __CGX2_SPRITE_H__
