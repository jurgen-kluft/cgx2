# 2D Graphics Fonts

A golang tool to convert TTF/OTF/BDF font files to a custom binary font pack

## Font-Pack

- Font Rendering: https://github.com/mcufont/mcufont
- Font Conversion: https://github.com/erkkah/tigrfont

Tool takes a .json file as input that describes the fonts to be included in the font pack and outputs a .bin file that contains the font and glyph data in the custom binary file format.

```json
{
  "files": [
    {
      "file": "font1.ttf",
      "fonts": [
         {
           "name": "font1x16",
           "size": 16,
           "chars": [
              "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m",
              "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z",
              "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M",
              "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
              "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", " ", 
              "%", "-", "+", ".", "°"
           ]
         },
         {
           "name": "font1x32",
           "size": 32,
           "chars": [
              "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m",
              "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z",
              "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M",
              "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
              "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", " ", 
              "%", "-", "+", ".", "°"
           ]
         }
      ]
    }
  ]
}
```

