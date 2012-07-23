#include <vector>
#include <array>
#include <iostream>
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>
#include <map>
#include "types.h"
#include "constants.h"
#include "pgm.h"
#include "hystograms.h"
#include "disjoint_set_forest.h"

using std::map;
using std::vector;
using std::min;
using std::max;
using std::pair;

typedef pair<int, int> ipair;

typedef vector<vector<bool> > bitonal_image;

struct ConnectedComponent {
    byte level;
    int left;
    int right;
    int top;
    int bottom;
    bitonal_image form;
    void save(FILE *file) {
        int height = form.size();
        int width = form[0].size();
        byte *colors = new byte[height * width];
        byte *pointer = colors;
        for (const auto &i : form) {
            for (const auto &j : i) {
                *(pointer++) = j;
            }
        }
        save_pbm(file, colors, width, height, width, height);
        delete colors;
    }
    ConnectedComponent(): left(999999), right(-1), top(999999), bottom(-1) {
    }
};

bitonal_image threshold(byte *pixels, int width, int height, byte level) {
    bitonal_image result(height, vector<bool> (width));
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            result[j][i] = pixels[j*width + i] <= level;
        }
    }
    return result;
}

vector<ConnectedComponent> find_connected_components(const bitonal_image &image) {
    int height = image.size();
    int width = image[0].size();
    vector<vector<int> > colors(height, vector<int> (width, 0));
    int active_color(1);
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            colors[i][j] = image[i][j] ? active_color++ : 0;
        }
    }
    std::cout << "ac: " << active_color << '\n';
    DisjointSetForest colors_forest(active_color);
    std::cout << "width: " << width << "; height: " << height << '\n';

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            for (int k = -1; k <= 1; ++k) {
                for (int l = -1; l <= 1; ++l) {
                    if (((i+k) >= 0) && ((i+k) < height) && ((j+l) >= 0) && ((j+l) < width) && image[i][j] && image[i+k][j+l]) {
                        colors_forest.unite(colors[i][j], colors[i+k][j+l]);
                    }
                }
            }
        }
    }
    std::cout << "coloring: OK\n";

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            colors[i][j] = colors_forest.find(colors[i][j]);
        }
    }
    map<int, int> colors_canonical;
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            if (colors[i][j]) {
                if (!colors_canonical.count(colors[i][j])) {
                    colors_canonical[colors[i][j]] = colors_canonical.size();
                }
                colors[i][j] = colors_canonical.at(colors[i][j]);
            }
        }
    }
    byte *pixels = new byte[height*width];
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            pixels[i*width + j] = colors[i][j] * 255/colors_canonical.size();
        }
    }
    std::cout << "Components: " << colors_canonical.size() << '\n';

    vector<ConnectedComponent> result(colors_canonical.size());
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            if (colors[i][j]) {
                ConnectedComponent &component = result[colors[i][j] - 1];
                component.left = min(component.left, i);
                component.right = max(component.right, i);
                component.top = min(component.top, j);
                component.bottom = max(component.bottom, j);
                component.level = colors[i][j];
            }
        }
    }
    for (auto &i : result) {
        i.form = bitonal_image(i.right - i.left + 1, vector<bool> (i.bottom - i.top + 1, false));
    }
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            if (colors[i][j]) {
                ConnectedComponent &component = result[colors[i][j] - 1];
                component.form.at(i - component.left).at(j - component.top) = true;
            }
        }
    }
    std::cout << "components: OK\n";

    for (auto &i: result) {
        char name[255];
        sprintf(name, "components/%d.pbm", i.level);
        i.save(fopen(name, "wb"));
    }
    save_pgm(fopen("test_connected.pgm", "wb"), pixels, height, width);
    std::cout << "forming: OK\n";
    return result;
}

int main(int argc, char *argv[]) {
    byte *pixels;
    int32 width, height, row_size, rows_count;
    FILE *data = fopen(argv[1], "rb");

    load_pgm(data, &width, &height, &row_size, &rows_count, &pixels, 0);

    fclose(data);

    vector<ConnectedComponent> components = find_connected_components(threshold(pixels, width, height, 128));

    return 0;
}
