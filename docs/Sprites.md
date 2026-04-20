# 2D Graphics Images

A golang tool to convert TGA/PNG image files to a custom binary file format that can be loaded by the library as an array of images

## TGA/PNG to Image-Pack

Tool takes a .json file as input that describes the images to be included in the image packand outputs a .bin file that contains the image data in the custom binary file format.

```json
{
  "files": [
    {
      "file": "sprite1.png",
      "images": [
         {
           "name": "sprite1",
           "format": "RGB565",
           "alpha": "A1"
         }
      ]
    },
    {
      "file": "sprite2.tga",
      "images": [
         {
           "name":"sprite2",
           "format": "RGB565",
           "alpha": "A1",
           "rect": {
               "x": 0,
               "y": 0,
               "w": 64,
               "h": 64
            }
         }
      ]
    }
  ]
}
```

- Image Pack File Format
  - Header
    - u64 magic (e.g. 'IMGPACK1')
    - u32 image count
    - Image[]
      - u16 width
      - u16 height
      - u16 format (e.g. RGBA5551, RGBA8888, 8-Bit color palette, 1-Bit, etc..)
      - u16 alpha format (e.g. A8, A4, A1, etc..) (for formats with separate alpha data)
      - u32 image data size
      - u64 image data offset in file
      - u32 alpha data size
      - u64 alpha data offset in file (for formats with separate alpha data)
      - u64 color palette offset in file (for paletted formats)
    - Image Data
      - pixel data
    - Color Palette Data
      - raw color palette data for each image (for paletted formats)
