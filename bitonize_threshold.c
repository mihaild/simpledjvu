#include "pgm.h"
#include <stdlib.h>
#include <stdio.h>
#include "types.h"

int main(int argc, char **argv) {
    FILE *in, *out;

    in = fopen(argv[1], "rb");
    out = fopen(argv[2], "wb");

    int level;

    level = atoi(argv[3]);

    byte *pixels;
    int32 width, height, row_size, rows_count;

    load_pgm(in, &width, &height, &row_size, &rows_count, &pixels, 0);

    int i, j;
    
    for (i = 0; i < height; ++i) {
        for (j = 0; j < width; ++j) {
            pixels[i * width + j] = (pixels[i * width + j]) < level;
        }
    }

    save_pbm(out, pixels, width, height, row_size, rows_count);

    fclose(in);
    fclose(out);

    return 0;
}
