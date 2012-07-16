#include "pgm.h"
#include <stdlib.h>
#include <stdio.h>
#include "types.h"

#define CANONICAL_BACKGROUND_LEVEL 220

byte canonize_level(byte bg, byte raw) {
    if ((raw <= bg) && (bg != 0)) {//мы типа в foreground
        return (byte)((int) CANONICAL_BACKGROUND_LEVEL * raw / bg);
    }
    else {//мы типа в background
        return (byte)((int) (raw - bg) * (255 - CANONICAL_BACKGROUND_LEVEL) / (255 - bg) + CANONICAL_BACKGROUND_LEVEL);
    }
}

void remove_background(byte *data_pixels, byte *background_pixels, byte *result_pixels, int32 width, int32 height) {
    int32 i, j;
    int32 target;
    for (i = 0; i < height; ++i) {
        for (j = 0; j < width; ++j) {
            target = i * width + j;
            /*result_pixels[target] = (background_pixels[target] > data_pixels[target]) ? (background_pixels[target] - data_pixels[target]) : 255;*/
            byte color;
            color = canonize_level(background_pixels[target], data_pixels[target]);
            /*color = max(data_pixels[target], 255-background_pixels[target]);*/
            result_pixels[target] = color;
        }
    }
}

int main(int argc, char **argv) {
    FILE *data, *background, *result;
    
    data = fopen(argv[1], "rb");
    background = fopen(argv[2], "rb");
    result = fopen(argv[3], "wb");

    byte *data_pixels, *background_pixels, *result_pixels;

    int32 width, height, row_size, rows_count;

    load_pgm(data, &width, &height, &row_size, &rows_count, &data_pixels, 0);
    load_pgm(background, &width, &height, &row_size, &rows_count, &background_pixels, 0);

    result_pixels = (byte *) malloc(width * height * sizeof(byte));

    remove_background(data_pixels, background_pixels, result_pixels, width, height);

    save_pgm(result, result_pixels, width, height);

    free(result_pixels);
    free(background_pixels);
    free(data_pixels);

    fclose(result);
    fclose(background);
    fclose(data);

    return 0;
}
