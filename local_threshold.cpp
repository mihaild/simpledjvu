#include <vector>
#include <array>
#include <iostream>
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>
#include "types.h"
#include "constants.h"
#include "pgm.h"
#include "hystograms.h"

int get_quantile(const Hystogram &hystogram, double level) {
    int sum = std::accumulate(hystogram.begin(), hystogram.end(), 0);
}

int main(int argc, char *argv[]) {
    FILE *data;

    data = fopen(argv[1], "rb");

    byte *pixels;
    int32 width, height, row_size, rows_count;

    load_pgm(data, &width, &height, &row_size, &rows_count, &pixels, 0);
    fclose(data);

    int radius = atoi(argv[2]);

    vector<vector<Hystogram> > rectangle_hystograms = get_rectangle_hystograms(pixels, row_size, rows_count);

    //for (int i = 

    return 0;
}
