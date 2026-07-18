# 2D Graphics Library

Mainly targetting ESP32 and displays with 16-bit RGB565 framebuffers.

- written in C++, very C like style
- comments are in C++ style (//), even multiline comments use (//)
- no std usage
- namespace is `ngx2` (no need to prefix all functions with `ngx2_` since we are using a namespace)
- system types are: u8, u16, u32, i8, i16, i32, f32, f64
- simple API for drawing shapes, text, and sprites
- all drawing functions must be deterministic and produce the same output given the same input
- offline tools are written in Go

## Assumptions

- framebuffer format is fixed as RGB565 
- origin (0, 0) is at the top-left corner of the framebuffer
- pixel coordinates are integer based (e.g. x = 10, y = 20)
- user allocates and owns any necessary buffers
  - library provides helper functions to calculate the required buffer sizes
- sprites
  - offline tool to convert image files to our binary format sprite pack
- fonts 
  - offline tool to convert TTF font files to our binary format font pack

## Design

- color (16-bit, 565)
- framedescr (width, height, format) (bytes_per_pixel(format))
- framebuffer
  - init_framebuffer (descr, pixel data)
  - clear_full_framebuffer (framebuffer, color)
- drawing functions:
  - draw_pixel (framebuffer_t& fb, rect_t& scissor, x, y, color)
  - draw_line (framebuffer_t& fb, rect_t& scissor, x start, y start, x end, y end, color)
  - draw_hline (framebuffer_t& fb, rect_t& scissor, x start, x end, y, color)
  - draw_vline (framebuffer_t& fb, rect_t& scissor, x, y start, y end, color)
  - draw_hdline (framebuffer_t& fb, rect_t& scissor, x start, x end, y, dash1 length, dash2 length, color)
  - draw_vdline (framebuffer_t& fb, rect_t& scissor, x, y start, y end, dash1 length, dash2 length, color)
  - draw_dline (framebuffer_t& fb, rect_t& scissor, x start, y start, x end, y end, dash1 length, dash2 length, color)
  - draw_arc (framebuffer_t& fb, rect_t& scissor, x, y, radius, start angle, end angle, color)
  - draw_circle (framebuffer_t& fb, rect_t& scissor, x, y, radius, fill, color)
  - draw_ellipse (framebuffer_t& fb, rect_t& scissor, x, y, radius x, radius y, fill, color)
  - draw_rectangle (framebuffer_t& fb, rect_t& scissor, x, y, width, height, fill, color)
  - draw_sprite (framebuffer_t& fb, rect_t& scissor, x, y)
  - draw_text (framebuffer_t& fb, rect_t& scissor, x, y, text)
- sprite pack:
  - sprite pack (array of sprite data, array max/size, etc..)
  - allocate_sprite_pack (allocate function, array of sprite data, array max/size)
  - load_sprite_from_custom_binary_file_format (e.g. RGBA5551, RGBA8888, 8-Bit color palette, 1-Bit, etc..)
  - golang tool to convert image files (e.g. PNG, TGA, JPG, BMP, etc..) to our binary format sprite pack
    - see go-gx2/cmd/pack-sprite for details
- font pack:
  - font pack (array of font and glyph data, array max/size)
  - allocate_font_pack (allocate function, array of font and glyph data, array max/size)
  - load_glyph_and_font_data_from_custom_binary_file_format
  - golang tool to convert TTF font files to our binary format font pack
    - see go-gx2/cmd/pack-font for details

