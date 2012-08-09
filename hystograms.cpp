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
Hystogram get_local_hystogram(const vector<vector<Hystogram> > &rectangles_hystogram, int side, int x, int y) {
    int left = ((x - side/2) > 0) ? (x - side/2) : 0;
    if ((left + side) >= rectangles_hystogram.size()) {
        left = rectangles_hystogram.size() - side - 1;
    }
    int top = ((y - side/2) > 0) ? (y - side/2) : 0;
    if ((top + side) >= rectangles_hystogram[left].size()) {
        top = rectangles_hystogram[left].size() - side - 1;
    }
    auto result = rectangles_hystogram[left][top] + rectangles_hystogram[left + side][top + side] - rectangles_hystogram[left][top + side] - rectangles_hystogram[left + side][top];
    for (int i = 0; i < result.size(); ++i) {
        assert(result[i] >= 0);
        assert(result[i] <= side*side);
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
