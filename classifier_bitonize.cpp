#include <vector>
#include <array>
#include <iostream>
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>
#include <unordered_map>
#include "types.h"
#include "constants.h"
#include "pgm.h"
#include "hystograms.h"
#include "disjoint_set_forest.h"

using std::unordered_map;
using std::vector;
using std::min;
using std::max;
using std::pair;

typedef pair<int, int> ipair;

typedef vector<vector<bool> > bitonal_image;

struct ConnectedComponent {
    int left;
    int right;
    int top;
    int bottom;
    int color;
    bitonal_image form;
    ConnectedComponent *parent;
    vector<ConnectedComponent *> childs;
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
    ConnectedComponent(): left(999999), right(-1), top(999999), bottom(-1), parent(NULL) {
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

vector<ConnectedComponent *> find_connected_components(const bitonal_image &image, vector<vector<int> > &colors, DisjointSetForest &colors_forest, vector<ConnectedComponent *> &prev_level) {
    int height = image.size();
    int width = image[0].size();

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

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            colors[i][j] = colors_forest.find(colors[i][j]);
        }
    }
    unordered_map<int, int> colors_canonical;
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            if (image[i][j]) {
                if (!colors_canonical.count(colors[i][j])) {
                    colors_canonical[colors[i][j]] = colors_canonical.size();
                }
            }
        }
    }

    /*byte *pixels = new byte[height*width];
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            pixels[i*width + j] = colors[i][j] ? (colors_canonical[colors[i][j]] * 255/colors_canonical.size()) : 0;
        }
    }
    save_pgm(fopen("coloring.pbm", "wb"), pixels, width, height);
    std::cout << "save coloring: OK\n";*/

    vector<ConnectedComponent *> result(colors_canonical.size());
    for (auto &i: result) {
        i = new ConnectedComponent();
    }
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            if (image[i][j]) {
                ConnectedComponent &component = *result[colors_canonical[colors[i][j]] - 1];
                component.left = min(component.left, i);
                component.right = max(component.right, i);
                component.top = min(component.top, j);
                component.bottom = max(component.bottom, j);
                component.color = colors[i][j];
            }
        }
    }

    for (auto &i : result) {
        i->form = bitonal_image(i->right - i->left + 1, vector<bool> (i->bottom - i->top + 1, false));
        for (auto &j : prev_level) {
            if (colors_forest.find(i->color) == colors_forest.find(j->color)) {
                /*j->childs.push_back(i);
                i->parent = j;*/
                i->childs.push_back(j);
                j->parent = i;
            }
        }
    }

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            if (image[i][j]) {
                ConnectedComponent &component = *result[colors_canonical[colors[i][j]] - 1];
                component.form.at(i - component.left).at(j - component.top) = true;
            }
        }
    }

    /*for (auto &i: result) {
        char name[255];
        sprintf(name, "components/%d.pbm", i.color);
        i.save(fopen(name, "wb"));
    }
    std::cout << "forming: OK\n";*/
    return result;
}

const int MIN_LEVEL = 10;
const int MAX_LEVEL = 250;
const int LEVEL_STEP = 15;
const int LEVELS = (MAX_LEVEL - MIN_LEVEL) / LEVEL_STEP + 1;

vector<vector<ConnectedComponent *> > build_connected_components_forest(byte *pixels, int width, int height) {
    vector<vector<ConnectedComponent *> > result(LEVELS);
    vector<vector<int> > colors(height, vector<int> (width, 0));
    int active_color(0);
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            colors[i][j] = active_color++;
        }
    }
    DisjointSetForest colors_forest(active_color);
    for (int i = 0, level = MIN_LEVEL; level <= MAX_LEVEL; ++i, level += LEVEL_STEP) {
        std::cerr << "Level: " << level << "; ";
        bitonal_image image = threshold(pixels, width, height, level);
        if (!i) {
            vector<ConnectedComponent *> tmp;
            result[i] = find_connected_components(image, colors, colors_forest, i == 0 ? tmp : result[i-1]);
        }
        else {
            result[i] = find_connected_components(image, colors, colors_forest, result[i-1]);
        }
        std::cerr << "components: " << (result[i].size()) << '\n';
    }
    return result;
}

int main(int argc, char *argv[]) {
    byte *pixels;
    int32 width, height, row_size, rows_count;
    FILE *data = fopen(argv[1], "rb");

    load_pgm(data, &width, &height, &row_size, &rows_count, &pixels, 0);

    fclose(data);

    //vector<ConnectedComponent> components = find_connected_components(threshold(pixels, width, height, 128));
    vector<vector<ConnectedComponent *> > connected_components_forest = build_connected_components_forest(pixels, width, height);

    return 0;
}
