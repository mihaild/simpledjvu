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

byte get_right_quantile(const Hystogram &hystogram, double level) {
    int need = std::accumulate(hystogram.begin(), hystogram.end(), 0) * level;
    int current_sum = 0;
    for (byte i = hystogram.size() - 1; i >= 0; --i) {
        current_sum += hystogram[i];
        if (current_sum >= need) {
            return i;
        }
    }
    return 0;
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

    byte *colors = new byte[width * height];

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            //std::cout << i << ' ' << j << ' ' << int(get_right_quantile(get_local_hystogram(rectangle_hystograms, radius, i, j), 0.5)) << ' ';
            //std::cout << int(get_right_quantile(get_local_hystogram(rectangle_hystograms, radius, i, j), 0.5)) << ' ';
            colors[i*width + j] = get_right_quantile(get_local_hystogram(rectangle_hystograms, radius, i, j), 0.05);
            //colors[i*width + j] = std::min(255, get_local_hystogram(rectangle_hystograms, radius, i, j)[150]);
            //colors[i*width + j] = j % 256;
        }
        //std::cout << '\n';
    }

    std::cout << "saving\n";
    save_pgm(fopen(argv[3], "wb"), colors, width, height);

    return 0;
}
