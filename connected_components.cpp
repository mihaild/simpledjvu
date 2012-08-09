#include <unordered_map>
#include <algorithm>
#include <iostream>
#include <string>
#include <queue>
#include <cmath>

#include <sys/stat.h>
#include <sys/types.h>

#include "disjoint_set_forest.h"
#include "pgm.h"
#include "connected_components.h"

#include "quality.h"

using std::unordered_map;
using std::min;
using std::max;
using std::queue;

double sqr(double x) {
    return x*x;
}

void ConnectedComponent::save(FILE *file) const {
    save_pbm(file, form);
}

ConnectedComponent::ConnectedComponent(): left(999999), right(-1), top(999999), bottom(-1) {
}

int ConnectedComponent::width() const {
    return right - left + 1;
}

int ConnectedComponent::height() const {
    return bottom - top + 1;
}

Point ConnectedComponent::mass_center() const {
    int x(0), y(0), n(0);
    for (int i = 0; i < height(); ++i) {
        for (int j = 0; j < width(); ++j) {
            x += form[i][j] * j;
            y += form[i][j] * i;
            n += form[i][j];
        }
    }
    return Point(x / n, y / n);
}

bool cmp_components_pointers(const ConnectedComponent *a, const ConnectedComponent *b) {
    return a->color < b->color;
}

vector<ConnectedComponent *> find_connected_components(const bitonal_image &image, vector<vector<int> > &colors, DisjointSetForest &colors_forest, vector<ConnectedComponent *> &prev_level, const vector<ipair> &new_points) {
    int height = image.size();
    int width = image[0].size();

    for (const auto &i : new_points) {
        int color = colors[i.first][i.second];
        if (i.first != 0 && image[i.first - 1][i.second]) {
            colors_forest.unite(color, colors[i.first - 1][i.second]);
        }
        if (i.first != height - 1 && image[i.first + 1][i.second]) {
            colors_forest.unite(color, colors[i.first + 1][i.second]);
        }
        if (i.second != 0 && image[i.first][i.second - 1]) {
            colors_forest.unite(color, colors[i.first][i.second - 1]);
        }
        if (i.second != 0 && image[i.first][i.second + 1]) {
            colors_forest.unite(color, colors[i.first][i.second + 1]);
        }
    }

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            colors[i][j] = colors_forest.find(colors[i][j]);
        }
    }

    vector<int> colors_canonical(colors_forest.size(), -1);
    int canonical_colors_count = 0;
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            if (image[i][j]) {
                if (colors_canonical[colors[i][j]] == -1) {
                    colors_canonical[colors[i][j]] = canonical_colors_count++;
                }
            }
        }
    }

    vector<ConnectedComponent *> tmp_result(canonical_colors_count);
    for (auto &i: tmp_result) {
        i = new ConnectedComponent();
    }
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            if (image[i][j]) {
                ConnectedComponent &component = *tmp_result[colors_canonical[colors[i][j]]];
                component.left = min(component.left, j);
                component.right = max(component.right, j);
                component.top = min(component.top, i);
                component.bottom = max(component.bottom, i);
                component.color = colors[i][j];
            }
        }
    }

    vector<ConnectedComponent *> result;

    for (int i = 0; i < tmp_result.size(); ++i) {
        if (tmp_result[i]->width() < MIN_WIDTH || tmp_result[i]->height() < MIN_HEIGHT) {
            colors_canonical[tmp_result[i]->color] = -1;
            delete tmp_result[i];
        }
        else {
            colors_canonical[tmp_result[i]->color] = result.size();
            result.push_back(tmp_result[i]);
        }
    }

    for (auto &i : result) {
        i->form = bitonal_image(i->height(), vector<bool> (i->width(), false));
    }
    for (int i = 0; i < prev_level.size(); ++i) {
        int color = colors_forest.find(prev_level[i]->color);
        if (colors_canonical[color] != -1) {
            result[colors_canonical[color]]->childs.push_back(i);
        }
    }

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            if (image[i][j] && colors_canonical[colors[i][j]] != -1) {
                ConnectedComponent &component = *result[colors_canonical[colors[i][j]]];
                component.form[i - component.top][j - component.left] = true;
            }
        }
    }

    return result;
}

ConnectedComponentForest build_connected_components_forest(byte *pixels, int width, int height) {
    vector<vector<ConnectedComponent *> > result;//(LEVELS);
    vector<vector<int> > colors(height, vector<int> (width, 0));
    int active_color(0);
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            colors[i][j] = active_color++;
        }
    }
    DisjointSetForest colors_forest(active_color);
    clock_t tmp = clock();
    array<vector<ipair>, LEVELS> pixels_by_level;
    GrayImage gray_image(height, vector<byte> (width));
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            gray_image[i][j] = pixels[i*width + j];
            pixels_by_level[gray_image[i][j] / LEVEL_STEP].push_back(ipair(i, j));
        }
    }
    bitonal_image image(height, vector<bool> (width, false));
    for (int i = 0, level = MIN_LEVEL; level <= MAX_LEVEL; ++i, level += LEVEL_STEP) {
        std::cerr << "Level: " << level << "; ";
        for (const auto &j : pixels_by_level[i]) {
            image[j.first][j.second] = true;
        }
        vector<ConnectedComponent *> components;
        if (result.empty()) {
            vector<ConnectedComponent *> tmp;
            components = find_connected_components(image, colors, colors_forest, tmp, pixels_by_level[i]);
        }
        else {
            components = find_connected_components(image, colors, colors_forest, result.back(), pixels_by_level[i]);
        }
        if (!components.empty()) {
            result.push_back(components);
        }
        std::cerr << "components: " << (components.size()) << '\n';
    }
    return ConnectedComponentForest(result, gray_image);
}

void ConnectedComponentForest::save_component(std::string path, int level, int number) const {
    FILE *f;
    char s[255];
    ConnectedComponent *component = components[level][number];
    mkdir(path.c_str(), 0777);
    sprintf(s, "%d", number);
    path += s;// + '/';
    //path += '/';
    mkdir(path.c_str(), 0777);
    f = fopen((path + ".pbm").c_str(), "wb");
    component->save(f);
    fclose(f);
    for (auto &i : component->childs) {
        save_component(path + "/", level-1, i);
    }
}

void ConnectedComponentForest::save(std::string path) const {
    for (int i = 0; i < components.back().size(); ++i) {
        save_component(path, components.size() - 1, i);
    }
}

double ConnectedComponentForest::component_quality(const ConnectedComponent &component) const {
    return -1./component.height();
}

vector<ConnectedComponent *> ConnectedComponentForest::get_best_subset() {
    vector<vector<int> > bifurcation_distance(components.size());
    vector<vector<int> > best_by_subtree(components.size());
    for (int i = 0; i < components.size(); ++i) {
        bifurcation_distance[i].resize(components[i].size(), 0);
        best_by_subtree[i].resize(components[i].size(), 0);
        for (int j = 0; j < components[i].size(); ++j) {
            ipair current(i, j);
            while (components[current.first][current.second]->childs.size() == 1) {
                current.second = components[current.first][current.second]->childs[0];
                --current.first;
                ++bifurcation_distance[i][j];
            }
            best_by_subtree[i][j] = bifurcation_distance[i][j];
            for (int k : components[current.first][current.second]->childs) {
                best_by_subtree[i][j] = max(best_by_subtree[i][j], best_by_subtree[current.first - 1][k]);
            }
        }
    }
    queue<ipair> take;
    for (int i = 0; i < best_by_subtree.back().size(); ++i) {
        take.emplace(best_by_subtree.size() - 1, i);
    }
    vector<ConnectedComponent *> result;
    while (!take.empty()) {
        ipair target = take.front();
        take.pop();
        ConnectedComponent *component = components[target.first][target.second];
        if (bifurcation_distance[target.first][target.second] && bifurcation_distance[target.first][target.second] >= best_by_subtree[target.first - 1][component->childs[0]] / 1.5) {
            result.push_back(component);
        }
        else {
            for (int i : component->childs) {
                take.emplace(target.first - 1, i);
            }
        }
    }
    std::cerr << "best components: OK\n";
    return result;
}

void place_components(const vector<ConnectedComponent *> components, bitonal_image &image) {
    for (const auto& component : components) {
        for (int i = 0; i < component->height(); ++i) {
            for (int j = 0; j < component->width(); ++j) {
                image[i + component->top][j + component->left] = component->form[i][j];
            }
        }
    }
}

const ConnectedComponent& ConnectedComponentForest::ConnectedComponentForestForwardIterator::operator *() {
    return *(forest.components[level][number]);
}
ConnectedComponentForest::ConnectedComponentForestForwardIterator& ConnectedComponentForest::ConnectedComponentForestForwardIterator::operator ++() {
    if (number < forest.components[level].size() - 1) {
        ++number;
    }
    else {
        number = 0;
        ++level;
    }
    return *this;
}
bool ConnectedComponentForest::ConnectedComponentForestForwardIterator::operator !=(const ConnectedComponentForestForwardIterator &other) const {
    return number != other.number || level != other.level;
}
ConnectedComponentForest::ConnectedComponentForestForwardIterator::ConnectedComponentForestForwardIterator(int level, int number, const ConnectedComponentForest &forest): level(level), number(number), forest(forest) {
}

ConnectedComponentForest::ConnectedComponentForestForwardIterator ConnectedComponentForest::begin() const {
    return ConnectedComponentForestForwardIterator(0, 0, *this);
}
ConnectedComponentForest::ConnectedComponentForestForwardIterator ConnectedComponentForest::end() const {
    return ConnectedComponentForestForwardIterator(components.size(), 0, *this);
}
