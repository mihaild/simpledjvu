#include "pgm.h"
#include <stdlib.h>
#include <stdio.h>
#include <cassert>
#include "types.h"

const int CANONICAL_BLACK_LEVEL = 30;
const int CANONICAL_WHITE_LEVEL = 225;

byte canonize_level(const byte raw, byte black, byte white) {
    if (raw <= black) {
        if (black == 0) {
            return CANONICAL_BLACK_LEVEL;
        }
        return CANONICAL_BLACK_LEVEL * raw / black;
    }
    if (raw >= white) {
        if (white == COLORS_COUNT - 1) {
            return CANONICAL_WHITE_LEVEL;
        }
        return CANONICAL_WHITE_LEVEL + (COLORS_COUNT - 1 - CANONICAL_WHITE_LEVEL) * (raw - white) / (COLORS_COUNT - 1 - white);
    }
    return CANONICAL_BLACK_LEVEL + (raw - black) * (CANONICAL_WHITE_LEVEL - CANONICAL_BLACK_LEVEL) / (white - black);
}

void normalize(byte *data_pixels, byte *black_pixels, byte *white_pixels, byte *result_pixels, int32 width, int32 height) {
    int32 i, j;
    int32 target;
    for (i = 0; i < height; ++i) {
        for (j = 0; j < width; ++j) {
            target = i * width + j;
            result_pixels[target] = canonize_level(data_pixels[target], black_pixels[target], white_pixels[target]);
        }
    }
}

int main(int argc, char **argv) {
    FILE *data, *black, *white, *result;
    
    data = fopen(argv[1], "rb");
    black = fopen(argv[2], "rb");
    white = fopen(argv[3], "rb");
    result = fopen(argv[4], "wb");

    byte *data_pixels, *black_pixels, *white_pixels, *result_pixels;

    int32 width, height, row_size, rows_count;

    load_pgm(data, &width, &height, &row_size, &rows_count, &data_pixels, 0);
    load_pgm(black, &width, &height, &row_size, &rows_count, &black_pixels, 0);
    load_pgm(white, &width, &height, &row_size, &rows_count, &white_pixels, 0);

    result_pixels = (byte *) malloc(width * height * sizeof(byte));

    normalize(data_pixels, black_pixels, white_pixels, result_pixels, width, height);

    save_pgm(result, result_pixels, width, height);

    free(result_pixels);
    free(white_pixels);
    free(black_pixels);
    free(data_pixels);

    fclose(result);
    fclose(white);
    fclose(black);
    fclose(data);

    return 0;
}
