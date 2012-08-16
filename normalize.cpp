#include <stdlib.h>
#include <stdio.h>

#include "djvulibre.h"

#include "normalize.h"
#include "types.h"


/*
 * hope, that black > white
 * else it can show strange picture, crash, or destroy the Earth
 */
byte canonize_level(const byte raw, const byte black, const byte white) {
    if (raw >= black) {
        if (black == COLORS_COUNT - 1) {
            return CANONICAL_BLACK_LEVEL;
        }
        return (COLORS_COUNT - 1 - CANONICAL_BLACK_LEVEL) * (raw - black) / (COLORS_COUNT - 1 - black) + CANONICAL_BLACK_LEVEL;
    }
    if (raw <= white) {
        if (white == 0) {
            return CANONICAL_WHITE_LEVEL;
        }
        return CANONICAL_WHITE_LEVEL * raw / white;
    }
    return CANONICAL_WHITE_LEVEL + (raw - white) * (CANONICAL_BLACK_LEVEL - CANONICAL_WHITE_LEVEL) / (black - white);
}

void normalize(const GBitmap &image, const GBitmap &black, const GBitmap &white, GBitmap &result) {
    int width = image.columns(), height = image.rows();
    result.init(height, width);
    result.set_grays(256);
    int32 i, j;
    int32 target;
    for (i = 0; i < height; ++i) {
        for (j = 0; j < width; ++j) {
            result[i][j] = canonize_level(image[i][j], black[i][j], white[i][j]);
        }
    }
}
