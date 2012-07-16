#include <vector>
#include <array>
#include "types.h"
#include "constants.h"

using std::vector;
using std::array;

Hystogram operator -(const Hystogram &a, const Hystogram &b) {
    Hystogram result(a);
    for (int i = 0; i < COLORS_COUNT; ++i) {
        result[i] -= b[i];
    }
    return result;
}

Hystogram operator +(const Hystogram &a, const Hystogram &b) {
    Hystogram result;
    for (int i = 0; i < COLORS_COUNT; ++i) {
        result[i] += b[i];
    }
    return result;
}

vector<vector<Hystogram> > get_rectangle_hystograms(byte *pixels, int row_size, int rows_count) {
    vector<vector<Hystogram> > rectangles_hystogram(rows_count, vector<Hystogram> (row_size));
    ++rectangles_hystogram[0][0][pixels[0]];
    for (int i = 0; i < rows_count; ++i) {
        if (i != 0) {
            rectangles_hystogram[i] = rectangles_hystogram[i-1];
        }
        for (int j = 0; j < row_size; ++j) {
            if (j != 0) {
                rectangles_hystogram[i][j] = rectangles_hystogram[i][j-1];
                ++rectangles_hystogram[i][j][pixels[i * row_size + j]];
            }
        }
    }
    return rectangles_hystogram;
}

// fails if radius > min(row_size, rows_count)
vector<vector<Hystogram> > get_local_hystograms(const vector<vector<Hystogram> > &rectangles_hystogram, int radius) {
    vector<vector<Hystogram> > result(rectangles_hystogram);
    for (int i = 0; i < rectangles_hystogram.size(); ++i) {
        int left = ((i - radius/2) > 0) ? (i - radius/2) : 0;
        for (int j = 0; j < rectangles_hystogram[i].size(); ++j) {
            int top = ((j - radius/2) > 0) ? (j - radius/2) : 0;
            result[i][j] = rectangles_hystogram[left][top] + rectangles_hystogram[left + radius][top + radius] - rectangles_hystogram[left][top + radius] - rectangles_hystogram[left + radius][top];
        }
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

int main(int argc, char *argw[]) {
    return 0;
}
