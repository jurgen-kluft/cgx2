#include "ccore/c_target.h"
#include "ccore/c_allocator.h"
#include "ccore/c_math.h"
#include "ccore/c_memory.h"

#include "cgx2/c_font.h"

#include <cmath>

namespace ncore
{
    namespace ngx2
    {
        // ============================================================================
        // Font System
        // ============================================================================

        font_context_t* new_font_context(const void* binary_data, u32 binary_size)
        {
            // Pointers replaced by u64 offsets from start of binary.
            // The format of the binary data:
            //   - [u64 offset to font 0 glyph data from start of binary]
            //   - [u32 font_count]
            //   - [u32 reserved]
            //   - font_t[font_count]

            font_context_t* ctx = (font_context_t*)binary_data;
            ctx->fonts          = (font_t*)((const u8*)binary_data + (u64)ctx->fonts);

            for (u32 i = 0; i < ctx->count; i++)
            {
                font_t* font  = &ctx->fonts[i];
                font->glyphs  = (glyph_t*)((const u8*)binary_data + (u64)font->glyphs);
                font->bitmaps = (const u8**)((const u8*)binary_data + (u64)font->bitmaps);
            }

            return ctx;
        }

        font_t* get_font(font_context_t* ctx, u32 font_id)
        {
            if (font_id >= ctx->count)
                return nullptr;
            return &ctx->fonts[font_id];
        }
    }  // namespace ngx2
}  // namespace ncore