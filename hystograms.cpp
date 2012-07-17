#include <vector>
#include <array>
#include <iostream>
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>
#include "types.h"
#include "constants.h"
#include "pgm.h"

#include <assert.h>

using std::vector;
using std::array;
using std::fill;

using std::cout;

Hystogram operator -(const Hystogram &a, const Hystogram &b) {
    Hystogram result(a);
    for (int i = 0; i < COLORS_COUNT; ++i) {
        result[i] -= b[i];
    }
    return result;
}

Hystogram& operator +=(Hystogram &a, const Hystogram &b) {
    for (int i = 0; i < COLORS_COUNT; ++i) {
        a[i] += b[i];
    }
    return a;
}

Hystogram operator +(const Hystogram &a, const Hystogram &b) {
    Hystogram result(a);
    result += b;
    return result;
}

vector<vector<Hystogram> > get_rectangle_hystograms(byte *pixels, int row_size, int rows_count) {
    vector<vector<Hystogram> > rectangles_hystogram(rows_count, vector<Hystogram> (row_size));
    ++rectangles_hystogram[0][0][pixels[0]];
    for (int i = 0; i < rows_count; ++i) {
        Hystogram cline;
        fill(cline.begin(), cline.end(), 0);
        for (int j = 0; j < row_size; ++j) {
            /*for (int k = 0; k <= pixels[i * row_size + j]; ++k) {
                ++cline[k];
            }*/
            ++cline[pixels[i * row_size + j]];
            rectangles_hystogram[i][j] = cline;
            if (i != 0) {
                rectangles_hystogram[i][j] += rectangles_hystogram[i-1][j];
            }
        }
    }
    return rectangles_hystogram;
}

// fails if radius > min(row_size, rows_count)
Hystogram get_local_hystogram(const vector<vector<Hystogram> > &rectangles_hystogram, int radius, int x, int y) {
    int left = ((x - radius/2) > 0) ? (x - radius/2) : 0;
    if ((left + radius) >= rectangles_hystogram.size()) {
        left = rectangles_hystogram.size() - radius - 1;
    }
    int top = ((y - radius/2) > 0) ? (y - radius/2) : 0;
    if ((top + radius) >= rectangles_hystogram[left].size()) {
        top = rectangles_hystogram[left].size() - radius - 1;
    }
    auto result = rectangles_hystogram[left][top] + rectangles_hystogram[left + radius][top + radius] - rectangles_hystogram[left][top + radius] - rectangles_hystogram[left + radius][top];
    for (int i = 0; i < result.size(); ++i) {
        assert(result[i] >= 0);
        assert(result[i] <= radius*radius);
    }
    return result;
}

LevelsDistribution get_levels_distribution(const Hystogram &hystogram) {
    LevelsDistribution result;
}

GlobalLevelsDistribution get_global_levels_distribution(byte *pixels, int row_size, int rows_count, int radius) {
    GlobalLevelsDistribution result(rows_count, vector<LevelsDistribution> (row_size));
    //vector<vector<Hystogram> > hystograms = get_hystograms(pixels, row_size, rows_count, radius);
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
    for (int i = radius / 2; i < width / 2; i += radius) {
        for (int j = radius / 2; j < height / 2; j += radius) {
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
