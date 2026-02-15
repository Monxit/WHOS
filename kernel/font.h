// kernel/font.h
// 8x16 bitmap font data

#ifndef FONT_H
#define FONT_H

#include "types.h"

#define FONT_WIDTH  8
#define FONT_HEIGHT 16

// Get font bitmap for a character (16 bytes, one per row)
const u8* font_get_char(char c);

#endif // FONT_H
