#include "ccore/c_target.h"
#include "ccore/c_allocator.h"
#include "ccore/c_math.h"
#include "ccore/c_memory.h"

#include "cgx2/c_sprite.h"
#include "cgx2/c_types.h"

#include <cmath>

namespace ncore
{
    namespace ngx2
    {
        // ============================================================================
        // Sprite System
        // ============================================================================

        sprite_context_t* new_sprite_context(const void* binary_data, u32 binary_size)
        {
            // Pointers replaced by u64 offsets from start of binary.
            // The format of the binary data:
            //   - [u64 offset to sprite 0 pixel data from start of binary]
            //   - [u32 sprite_count]
            //   - sprite_t[sprite_count]

            sprite_context_t* ctx = (sprite_context_t*)binary_data;
            ctx->sprites          = (sprite_t*)((const u8*)binary_data + (u64)ctx->sprites);

            for (u32 i = 0; i < ctx->count; i++)
            {
                sprite_t* sprite   = &ctx->sprites[i];
                sprite->pixel_data = ((const u8*)binary_data + (u64)sprite->pixel_data);
                if (sprite->alpha_data != 0)
                    sprite->alpha_data = ((const u8*)binary_data + (u64)sprite->alpha_data);
                if (sprite->palette_data != 0)
                    sprite->palette_data = (const color_t*)((const u8*)binary_data + (u64)sprite->palette_data);
            }

            return ctx;
        }

        sprite_t* get_sprite(sprite_context_t* ctx, u32 sprite_id)
        {
            if (sprite_id >= ctx->count)
                return nullptr;
            return &ctx->sprites[sprite_id];
        }
    }
}