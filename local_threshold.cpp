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

    char *input, *output;
    int radius;
    double level;
    if (argc < 3 || argc > 5) {
        std::cerr << "Wrong usage\n";
        return 1;
    }
    input = argv[1];
    output = argv[2];

    radius = (argc >= 4) ? atoi(argv[3]) : 20;
    level = (argc >= 5) ? atof(argv[4]) : 0.05;

    data = fopen(input, "rb");

    byte *pixels;
    int32 width, height, row_size, rows_count;

    load_pgm(data, &width, &height, &row_size, &rows_count, &pixels, 0);
    fclose(data);

    vector<vector<Hystogram> > rectangle_hystograms = get_rectangle_hystograms(pixels, row_size, rows_count);

    byte *colors = new byte[width * height];

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            //std::cout << i << ' ' << j << ' ' << int(get_right_quantile(get_local_hystogram(rectangle_hystograms, radius, i, j), 0.5)) << ' ';
            //std::cout << int(get_right_quantile(get_local_hystogram(rectangle_hystograms, radius, i, j), 0.5)) << ' ';
            colors[i*width + j] = get_right_quantile(get_local_hystogram(rectangle_hystograms, radius, i, j), level);
            //colors[i*width + j] = std::min(255, get_local_hystogram(rectangle_hystograms, radius, i, j)[150]);
            //colors[i*width + j] = j % 256;
        }
        //std::cout << '\n';
    }

    std::cout << "saving\n";
    save_pgm(fopen(output, "wb"), colors, width, height);

    return 0;
}
