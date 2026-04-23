#ifndef __CGX2_SPRITE_H__
#define __CGX2_SPRITE_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
    #pragma once
#endif

namespace ncore
{
    namespace ngx2
    {
        // ============================================================================
        // Sprite System
        // ============================================================================

        struct sprite_t
        {
            u16            width;
            u16            height;
            u16            format;
            u16            reserved;
            u32            pixel_data_size;
            u32            alpha_data_size;
            const void*    pixel_data;
            const void*    alpha_data;
            const color_t* palette_data;  // always u32[256] RGBA8888 palette, used only if pixel_format is indexed
        };

        struct sprite_context_t
        {
            sprite_t* sprites;
            u32       count;
            u32       reserved;  // padding to make sizeof(sprite_context_t) a multiple of 8
        };

        
        // ============================================================================
        // Sprites
        // ============================================================================

        sprite_context_t* new_sprite_context(const void* binary_data, u32 binary_size);
        sprite_t*         get_sprite(sprite_context_t* ctx, u32 sprite_id);

    }  // namespace ngx2
}  // namespace ncore

#endif  /// __CGX2_SPRITE_H__
