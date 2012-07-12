#include "pgm.h"
#include <stdlib.h>
#include <stdio.h>

typedef int int32;
typedef unsigned char byte;

int main(int argc, char **argv) {
    FILE *data, *background, *result;
    
    data = fopen(argv[1], "rb");
    background = fopen(argv[2], "rb");
    result = fopen(argv[3], "wb");

    byte *data_pixels, *background_pixels, *result_pixels;

    int32 width, height, row_size, rows_count;

    load_pgm(data, &width, &height, &row_size, &rows_count, &data_pixels, 0);
    load_pgm(background, &width, &height, &row_size, &rows_count, &background_pixels, 0);

    return 0;
}
