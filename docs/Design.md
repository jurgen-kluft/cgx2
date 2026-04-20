# 2D Graphics Library

- written in C++, but very C like
- comments are in C++ style (//), even multiline comments use (//)
- no std usage
- namespace is `ngx2` (no need to prefix all functions with `ngx2_` since we are using a namespace)
- system types are: u8, u16, u32, i8, i16, i32, f32, f64
- internal structures are hidden from the user
- simple API for drawing shapes, text, and sprites
- all drawing functions must be deterministic and produce the same output given the same input

## Focus

## Assumptions

- framebuffer format is fixed as RGBA8888 (32 bits per pixel, 8 bits for each channel)
- framebuffer can be quantized to other formats (e.g. RGB565, RGBA5551, etc..)
  - quantization is done by the library, user just needs to provide a target framebuffer with the desired format
  - quantization must be deterministic and produce the same output given the same input
- origin (0, 0) is at the top-left corner of the framebuffer
- pixel coordinates are integer based (e.g. x = 10, y = 20)
- user provides an allocate function, the library doesn't do any allocation/deallocation itself
- all loaded images are in RGBA8888 format

## Blending

```c
out.rgb = (src.rgb * α + dst.rgb * (255 − α)) / 255;
out.a   = α + dst.a * (255 − α) / 255;

struct blend_state_t
{
    u8 alpha;              // 0..255
    u8 ignore_src_alpha;   // 0 or 1
};

// src_alpha ∈ [0,255]
// ctx.alpha ∈ [0,255]
// result is rounded deterministically

if (ignore_src_alpha == 0) {
    α = (src_alpha * ctx.alpha) / 255;
} else {
    α = ctx.alpha;
}

```

## Design

- color (32-bit, r, g, b, a)
- framebuffer (width, height, bpp, format)
  - allocate_framebuffer (allocate function, width, height, bpp, format)
  - clear_full_framebuffer (ctx, color)
  - quantize_framebuffer (ctx, target format, target framebuffer)
- context (framebuffer, clip rect, font context, sprite context, etc..)
  - allocate_context (allocate function, framebuffer, clip rect, font context, sprite context, etc..)
  - begin_frame (ctx, framebuffer, clip rect)
    - push state 
      - first state:
        - scissor=framebuffer-size
        - color=white (255, 255, 255, 255)
        - blend state=alpha blending enabled
        - sa=255
        - sprite=default
        - font=default
        - fill=none
        - rotation angle=0.0
        - scale=1.0
      - set color
      - set blend state
      - set scissor rect
      - set sprite
      - set font
      - set fill
      - set rotation angle
      - set scale
    - pop state
  - end_frame (ctx)
- drawing, per frame-buffer format we have the following drawing functions:
  - draw_pixel (ctx, x, y)
  - draw_line (ctx, x start, y start, x end, y end)
  - draw_hline (ctx, x start, x end, y)
  - draw_vline (ctx, x, y start, y end)
  - draw_hdline (ctx, x start, x end, y, dash1 length, dash2 length)
  - draw_vdline (ctx, x, y start, y end, dash1 length, dash2 length)
  - draw_dline (ctx, x start, y start, x end, y end, dash1 length, dash2 length)
  - draw_arc (ctx, x, y, radius, start angle, end angle)
  - draw_circle (ctx, x, y, radius)
  - draw_ellipse (ctx, x, y, radius x, radius y)
  - draw_rectangle (ctx, x, y, width, height)
- sprite rendering:
  - sprite context (array of sprite data, array max/size, etc..)
  - allocate_sprite_context (allocate function, array of sprite data, array max/size)
  - load sprite from custom binary file format (e.g. RGBA5551, RGBA8888, 8-Bit color palette, 1-Bit, etc..)
  - golang tool to convert image files (e.g. PNG, BMP, etc..) to our binary format sprite pack
  - draw_sprite (ctx, x, y)
- font rendering:
  - font context (array of font and glyph data, array max/size)
  - allocate_font_context (allocate function, array of font and glyph data, array max/size)
  - load glyph and font data from custom binary file format
  - golang tool to convert TTF font files to our binary format font pack
  - draw_text (ctx, x, y, text)
