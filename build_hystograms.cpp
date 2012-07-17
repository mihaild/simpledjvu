#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <algorithm>
#include "types.h"
#include "constants.h"
#include "pgm.h"
#include "hystograms.h"

using std::cout;

int main(int argc, char *argv[]) {
    FILE *data;

    data = fopen(argv[1], "rb");

    byte *pixels;
    int32 width, height, row_size, rows_count;

    load_pgm(data, &width, &height, &row_size, &rows_count, &pixels, 0);
    fclose(data);

    int radius = atoi(argv[2]);

    vector<vector<Hystogram> > rectangle_hystograms = get_rectangle_hystograms(pixels, row_size, rows_count);

    std::cout << "hystograms OK\n";

    int max_hysto = 0;
    for (int i = radius / 2; i < width; i += radius) {
        for (int j = radius / 2; j < height; j += radius) {
            Hystogram hystogram = get_local_hystogram(rectangle_hystograms, radius, i, j);
            for (int k = 0; k < COLORS_COUNT; ++k) {
                max_hysto = (max_hysto > hystogram[k]) ? max_hysto : hystogram[k];
            }
        }
    }
    int canonical_max_hysto = max_hysto;

    const int COLORS_SCALE = 4;
    const int LEVEL_SCALE = 8;
    int picture_size = COLORS_COUNT*COLORS_SCALE*(canonical_max_hysto/LEVEL_SCALE);
    byte *colors = new byte[picture_size];
    for (int i = radius / 2; i < width; i += radius) {
        for (int j = radius / 2; j < height; j += radius) {
            std::cout << i << ' ' << j << "\n";
            Hystogram hystogram = get_local_hystogram(rectangle_hystograms, radius, i, j);
            std::reverse(hystogram.begin(), hystogram.end());

            max_hysto = *std::max_element(hystogram.begin(), hystogram.end());
            for (int k = 0; k < picture_size; ++k) {
                colors[k] = 0;
            }

            for (int k = 0; k < COLORS_COUNT; ++k) {
                if (hystogram[k]) {
                    cout << k << ' ' << hystogram[k] << '\n';
                }
                for (int l = 0; l < (canonical_max_hysto / max_hysto) * hystogram[k] / LEVEL_SCALE; ++l) {
                    for (int p = 0; p < COLORS_SCALE; ++p) {
                        colors[picture_size - (k*COLORS_SCALE + p + l*COLORS_COUNT*COLORS_SCALE)] = 1;
                        //colors[(k*COLORS_SCALE + p + l*COLORS_COUNT*COLORS_SCALE)] = 1;
                    }
                }
            }
            std::cout << "ok\n";

            char fname[255];
            sprintf(fname, "test_hystograms/%s_%d_%d.pbm", argv[3], i, j);
            std::cout << fname << '\n';
            FILE *out(fopen(fname, "wb"));
            std::cout << "saving...\n";
            save_pbm(out, colors, COLORS_COUNT*COLORS_SCALE, canonical_max_hysto/LEVEL_SCALE, COLORS_COUNT*COLORS_SCALE, canonical_max_hysto/LEVEL_SCALE);
            cout << "closing...\n";
            fclose(out);
            std::cout << "saved\n";
        }
    }
    delete colors;

    return 0;
}
