#include "ccore/c_target.h"
#include "ccore/c_math.h"
#include "ccore/c_memory.h"

#include "cgx2/c_effects.h"
#include "cgx2/c_types.h"

#include <cmath>

namespace ncore
{
    namespace ngx2
    {
        // Minimal LCG Linear Congruential Generator for deterministic random numbers
        inline i32 s_custom_rand(u32& rand_state, i32 min, i32 max)
        {
            rand_state = rand_state * 1103515245 + 12345;
            u32 val    = (rand_state / 65536) % 32768;
            return min + (val % (max - min + 1));
        }

        // Initialize the 8-bit color lookup table
        void fire_effect_init(fire_effect_t& effect, u8* fire_grid, u16 fire_width, u16 fire_height, u16* palette)
        {
            effect.fire_grid   = fire_grid;
            effect.palette     = palette;
            effect.rand_state  = 12345;
            effect.fire_width  = fire_width;
            effect.fire_height = fire_height;

            for (i32 i = 0; i < 256; i++)
            {
                u8 r = 0, g = 0, b = 0;

                if (i < 85)
                {  // Phase 1: Deep red to bright red
                    r = i * 3;
                }
                else if (i < 170)
                {  // Phase 2: Red transitions to Orange and Yellow
                    r = 255;
                    g = (i - 85) * 3;
                }
                else
                {  // Phase 3: Yellow transitions to White
                    r = 255;
                    g = 255;
                    b = (i - 170) * 3;
                }

                // Pack into a 16-bit pixel value (Format: RGB565)
                // Adjust shifts depending on the window framework's expected pixel format
                effect.palette[i] = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
            }

            // Explicitly zero out the initial fire grid
            for (i32 i = 0; i < fire_width * fire_height; i++)
            {
                effect.fire_grid[i] = 0;
            }
        }

        // Run the simulation core for a single frame
        static void s_update_fire(fire_effect_t& effect)
        {
            // Inject maximum heat energy into the bottom row
            i32 bottom_row_offset = (effect.fire_height - 1) * effect.fire_width;
            for (i32 x = 0; x < effect.fire_width; x++)
            {
                effect.fire_grid[bottom_row_offset + x] = s_custom_rand(effect.rand_state, 160, 255);
            }

            // Propagate energy upward through the grid matrix
            for (i32 y = 0; y < effect.fire_height - 1; y++)
            {
                for (i32 x = 1; x < effect.fire_width - 1; x++)
                {
                    i32 current_idx = y * effect.fire_width + x;

                    // Look directly at pixels beneath the current position
                    i32 below     = current_idx + effect.fire_width;
                    i32 below_l   = below - 1;
                    i32 below_r   = below + 1;
                    i32 below_two = (y < effect.fire_height - 2) ? below + effect.fire_width : below;

                    // Compute neighbor heat average
                    i32 total = (i32)effect.fire_grid[below_l] + (i32)effect.fire_grid[below] + (i32)effect.fire_grid[below_r] + (i32)effect.fire_grid[below_two];

                    // Attenuate heat using a cooling decay factor
                    i32 decay   = s_custom_rand(effect.rand_state, 0, 2);
                    i32 new_val = (total >> 2) - decay;  // Bitwise shift right by 2 replaces dividing by 4

                    // Clamp results to bounds
                    if (new_val < 0)
                        new_val = 0;

                    // Write results slightly shifted upward (current_idx - effect.fire_width) to move the flame
                    // Subtracting custom_rand adjustments here can create wind turbulence effects
                    if (current_idx >= effect.fire_width)
                    {
                        effect.fire_grid[current_idx - effect.fire_width] = (u8)new_val;
                    }
                }
            }
        }

        // Render internal buffer to screen array
        void s_render_to_buffer(fire_effect_t& effect, u16* fb, u16 fb_width, u16 fb_height)
        {
            u16* fb_row = fb + ((u32)fb_width * (fb_height - 1));
            fb_row += (fb_width - effect.fire_width) / 2;  // Center the fire effect horizontally

            u8* fire_row = effect.fire_grid + ((u32)effect.fire_width * (effect.fire_height - 1));
            for (i32 y = 0; y < effect.fire_height - 1; y++)
            {
                u16*      row      = fb_row;
                u8*       fire     = fire_row;
                u8 const* fire_end = fire + effect.fire_width;
                while (fire < fire_end)
                {
                    u8 fire_value = *fire++;
                    *row++        = effect.palette[fire_value];
                }
                fb_row -= fb_width;             // Move to the previous row in framebuffer
                fire_row -= effect.fire_width;  // Move to the previous row in fire grid
            }
        }

        // Main execution frame function loop called by the operating system platform loop
        void fire_effect_process_frame(fire_effect_t& effect, u16* fb, u16 fb_width, u16 fb_height)
        {
            s_update_fire(effect);
            s_render_to_buffer(effect, fb, fb_width, fb_height);
        }

    }  // namespace ngx2
}  // namespace ncore
