# Fonts

## Support

Fonts currently support non-variable .ttf files.

## Caching
Fonts get cached as an image file (.png) and a binary file  
containing information on the glyphs.  

**Filepath format:** `[FONT_NAME].ttf_[START]_[END]_[PT].bin`
```ini
[FONT_NAME] The filename that was used to load the font.
[START]     The begin range of the font glyphs. (ie. ascii would be 32-128)
[END]       The end range of the font glyphs.
[PT]        The pt size of the font when loaded. 
```

**File format:**
```ini
[GLYPH_COUNT]     8  bytes unsigned integer
[TEXT_HEIGHT]     4  bytes float
[GLYPHS...]       56 bytes * [GLYPH_COUNT]
    [advance]     4  bytes unsigned integer
    [offset]      4  bytes unsigned integer
    [bearing]     8  bytes float vec2
    [size]        8  bytes float vec2
    [texcoords]   32 bytes float vec2[4]
```