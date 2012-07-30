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
            for (int k = 0; k <= 1; ++k) {
                for (int l = 0; l <= 1; ++l) {
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
                    colors_canonical[colors[i][j]] = colors_canonical.size() - 1;
                }
            }
        }
    }

    vector<ConnectedComponent *> tmp_result(colors_canonical.size());
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
            colors_canonical.erase(tmp_result[i]->color);
            delete tmp_result[i];
        }
        else {
            colors_canonical[tmp_result[i]->color] = result.size();
            result.push_back(tmp_result[i]);
        }
    }

    for (auto &i : result) {
        i->form = bitonal_image(i->height(), vector<bool> (i->width(), false));
        for (int j = 0; j < prev_level.size(); ++j) {
            if (colors_forest.find(i->color) == colors_forest.find(prev_level[j]->color)) {
                i->childs.push_back(j);
            }
        }
    }

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            if (image[i][j] && colors_canonical.count(colors[i][j])) {
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
    for (int i = 0, level = MIN_LEVEL; level <= MAX_LEVEL; ++i, level += LEVEL_STEP) {
        std::cerr << "Level: " << level << "; ";
        bitonal_image image = threshold(pixels, width, height, level);
        vector<ConnectedComponent *> components;
        if (result.empty()) {
            vector<ConnectedComponent *> tmp;
            components = find_connected_components(image, colors, colors_forest, tmp);
        }
        else {
            components = find_connected_components(image, colors, colors_forest, result.back());
        }
        if (!components.empty()) {
            result.push_back(components);
        }
        std::cerr << "components: " << (components.size()) << '\n';
    }
    GrayImage image(height, vector<byte> (width));
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            image[j][i] = pixels[j*width + i];
        }
    }
    return ConnectedComponentForest(result, image);
}

void ConnectedComponentForest::save_component(std::string path, int level, int number) const {
    FILE *f;
    char s[255];
    ConnectedComponent *component = components[level][number];
    mkdir(path.c_str(), 0777);
    sprintf(s, "%d", component->color);
    path += s;// + '/';
    path += '/';
    mkdir(path.c_str(), 0777);
    f = fopen((path + "image.pgm").c_str(), "wb");
    component->save(f);
    fclose(f);
    for (auto &i : component->childs) {
        save_component(path, level-1, i);
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
    //vector<HystogramQualifier> qualifiers;
    vector<QualifierInterface *> qualifiers;
    ExternalFeautureGetter height_feature_getter(*this, feature_height);
    ExternalFeautureGetter width_feature_getter(*this, feature_width);
    qualifiers.push_back(new HystogramQualifier(&height_feature_getter, *this));
    qualifiers.push_back(new HystogramQualifier(&width_feature_getter, *this));
    qualifiers.push_back(new GradientFeatureGetter(*this));
    //GradientFeatureGetter gradient_feature_getter(*this);
    //std::cout << gradient_feature_getter.min() << ' ' << gradient_feature_getter.max() << '\n';
    std::cerr << "qualifier: OK\n";

    vector<vector<double > > qualities(components.size());
    for (int i = 0; i < components.size(); ++i) {
        qualities[i].resize(components[i].size());
        for (int j = 0; j < components[i].size(); ++j) {
            qualities[i][j] = 0.0;
            for (const auto &qualifier : qualifiers) {
                qualities[i][j] += qualifier->quality(*components[i][j]);
            }
            //qualities[i][j] *= gradient_feature_getter.get_feature(*components[i][j]);// - gradient_feature_getter.max() / 4;
        }
    }
    std::cerr << "qualities: OK\n";

    vector<vector<double> > best_by_subtree(components.size());
    for (int i = 0; i < best_by_subtree.size(); ++i) {
        best_by_subtree[i].resize(components[i].size());
        for (int j = 0; j < best_by_subtree[i].size(); ++j) {
            double cur_best_by_subtree(0);
            for (int k : components[i][j]->childs) {
                cur_best_by_subtree += max(best_by_subtree[i - 1][k], qualities[i - 1][k]);
            }
            best_by_subtree[i][j] = cur_best_by_subtree;
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
        if (best_by_subtree[target.first][target.second] > qualities[target.first][target.second]) {
            for (int i : components[target.first][target.second]->childs) {
                take.emplace(target.first - 1, i);
            }
        }
        else {
            result.push_back(components[target.first][target.second]);
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
