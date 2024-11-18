/*
 To create and/or view animations/frames, use the following website:
 https://xantorohara.github.io/led-matrix-editor/
*/
#pragma once

#include <stdint.h>
#include "matrixDisplay.h"

#define TOTAL_NUM_OF_CHARACRERS  (ALPHABET_COUNT + SPECIAL_CHARACTERS_COUNT)

typedef struct {
    uint64_t graphic;
    char character;
} graphicCharMap_t;

/*-----------------------------------------------------------
GRAPHICS
------------------------------------------------------------*/

//A - Z (MUST BE IN ORDER)
const graphicCharMap_t graphicCharMap[] = {
    // Letters
    {0x0033333f33331e0c, 'A'},
    {0x003f66663e66663f, 'B'},
    {0x003c66030303663c, 'C'},
    {0x001f36666666361f, 'D'},
    {0x007f46161e16467f, 'E'},
    {0x000f06161e16467f, 'F'},
    {0x007c66730303663c, 'G'},
    {0x003333333f333333, 'H'},
    {0x001e0c0c0c0c0c1e, 'I'},
    {0x001e333330303078, 'J'},
    {0x006766361e366667, 'K'},
    {0x007f66460606060f, 'L'},
    {0x0063636b7f7f7763, 'M'},
    {0x006363737b6f6763, 'N'},
    {0x001c36636363361c, 'O'},
    {0x000f06063e66663f, 'P'},
    {0x00381e3b3333331e, 'Q'},
    {0x006766363e66663f, 'R'},
    {0x001e33380e07331e, 'S'},
    {0x001e0c0c0c0c2d3f, 'T'},
    {0x003f333333333333, 'U'},
    {0x000c1e3333333333, 'V'},
    {0x0063777f6b636363, 'W'},
    {0x0063361c1c366363, 'X'},
    {0x001e0c0c1e333333, 'Y'},
    {0x007f664c1831637f, 'Z'},

    // Special Characters
    {0x0000003c00000000, '-'},
    {0x3c42a59999a5423c, '%'}  // circle with X
};
static_assert(TOTAL_NUM_OF_CHARACRERS == (sizeof(graphicCharMap) / sizeof(graphicCharMap_t)), "TOTAL_NUM_OF_CHARACRERS does not match the number of elements in graphicCharMap");

//WORDn\nSEEK!
const uint64_t dispWordNSeek[] = {
    0x0063777f6b636363,
    0x001c36636363361c,
    0x006766363e66663f,
    0x001f36666666361f,
    0x006666667e3e0000,
    0x001e33380e07331e,
    0x007f46161e16467f,
    0x007f46161e16467f,
    0x006766361e366667,
    0x180018183c3c1800
};

//-----/n<ZAB>
const uint64_t dispEmptyBoard[] = {
    0x0000003c00000000,
    0x0000003c00000000,
    0x0000003c00000000,
    0x0000003c00000000,
    0x0000003c00000000,
    0x60787e7f7e786000,
    0x007f664c1831637f,
    0x0033333f33331e0c,
    0x003f66663e66663f,
    0x061e7efe7e1e0600
};



/*-----------------------------------------------------------
ANIMATIONS
------------------------------------------------------------*/

const uint64_t bootAnimation[] = {
    0xffffffffffffffff,
    0xfff9c1bdffffdbff,
    0x00063e4200002400
};