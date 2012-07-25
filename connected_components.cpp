#include <unordered_map>
#include <algorithm>
#include <iostream>

#include <sys/stat.h>
#include <sys/types.h>

#include "disjoint_set_forest.h"
#include "pgm.h"
#include "connected_components.h"

using std::unordered_map;
using std::min;
using std::max;

void ConnectedComponent::save(FILE *file) {
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

ConnectedComponent::ConnectedComponent(): left(999999), right(-1), top(999999), bottom(-1), parent(NULL) {
}

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

    return result;
}

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

void save_component(ConnectedComponent component, std::string path) {
    FILE *f;
    char s[255];
    if (component.right - component.left < 4 || component.bottom - component.top < 4) {
        return;
    }
    mkdir(path.c_str(), 0777);
    sprintf(s, "%d", component.color);
    path += s;
    f = fopen((path + ".pgm").c_str(), "wb");
    component.save(f);
    fclose(f);
    path += "/";
    for (auto &i : component.childs) {
        save_component(*i, path);
    }
}
