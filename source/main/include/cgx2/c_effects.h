#ifndef __CGX2_EFFECTS_H__
#define __CGX2_EFFECTS_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
    #pragma once
#endif

#include "cgx2/c_types.h"

namespace ncore
{
    namespace ngx2
    {
        struct fire_effect_t
        {
            u32  rand_state;   // Random number generator state for deterministic randomness
            u8*  fire_grid;    // 8-bit[fire_width * fire_height] heat values for each pixel in the fire effect
            u16  fire_width;   // Width of the fire effect grid
            u16  fire_height;  // Height of the fire effect grid
            u16* palette;      // color palette mapping heat values to RGB565 colors
        };

        void fire_effect_init(fire_effect_t& effect, u8* fire_grid, u16 fire_width, u16 fire_height, u16* palette);
        void fire_effect_process_frame(fire_effect_t& effect, u16* fb, u16 fb_width, u16 fb_height);

    }  // namespace ngx2
}  // namespace ncore

#endif  /// __CGX2_EFFECTS_H__
