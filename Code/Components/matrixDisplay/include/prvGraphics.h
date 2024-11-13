/*
 To create and/or view animations/frames, use the following website:
 https://xantorohara.github.io/led-matrix-editor/
*/
#pragma once

#include <stdint.h>

typedef enum
{
    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,

    ALPHABET_COUNT
} alphabet_t;

/*-----------------------------------------------------------
GRAPHICS
------------------------------------------------------------*/

//A - Z (MUST BE IN ORDER)
const uint64_t graphicsLetter[] = {
    0x0033333f33331e0c,
    0x003f66663e66663f,
    0x003c66030303663c,
    0x001f36666666361f,
    0x007f46161e16467f,
    0x000f06161e16467f,
    0x007c66730303663c,
    0x003333333f333333,
    0x001e0c0c0c0c0c1e,
    0x001e333330303078,
    0x006766361e366667,
    0x007f66460606060f,
    0x0063636b7f7f7763,
    0x006363737b6f6763,
    0x001c36636363361c,
    0x000f06063e66663f,
    0x00381e3b3333331e,
    0x006766363e66663f,
    0x001e33380e07331e,
    0x001e0c0c0c0c2d3f,
    0x003f333333333333,
    0x000c1e3333333333,
    0x0063777f6b636363,
    0x0063361c1c366363,
    0x001e0c0c1e333333,
    0x007f664c1831637f
};
static_assert(ALPHABET_COUNT == (sizeof(graphicsLetter) / sizeof(uint64_t)), "ALPHABET_COUNT does not match the number of elements in graphicsLetter");

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