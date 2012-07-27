#include <algorithm>
#include <iostream>
#include <stdexcept>

#include "connected_components.h"
#include "quality.h"

using std::min;
using std::max;

HystogramQualifier::HystogramQualifier(AbstractFeatureGetter *feature_getter, const ConnectedComponentForest &forest): feature_getter(feature_getter) {
    int size = feature_getter->max() - feature_getter->min() + 1;
    hystogram.resize(size);
    hystogram_sums.resize(size);
    std::cout << "min: " << feature_getter->min() << '\n';
    for (const auto &component : forest) {
        ++hystogram.at(feature_getter->get_feature(component) - feature_getter->min());
    }
    int res(0);
    for (int i = 0; i < size; ++i) {
        res += hystogram[i];
        hystogram_sums[i] = res;
    }
    std::cout << "hystogram: ok\n";
}

double HystogramQualifier::quality(const ConnectedComponent &component) const {
    int value = feature_getter->get_feature(component);
    int left_sum = hystogram_sums[value - feature_getter->min()];
    int right_sum = hystogram_sums.back() - left_sum;
    if (left_sum && right_sum) {
        return static_cast<double> (min(left_sum, right_sum)) / max(left_sum, right_sum);
    }
    return -1.0;
}

int black_area(const ConnectedComponent &component) {
    return component.width() * component.height();
    int res(0);
    for (const auto &i : component.form) {
        for (const auto &j : i) {
            res += j;
        }
    }
    return res;
}

ExternalFeautureGetter::ExternalFeautureGetter(const ConnectedComponentForest &forest, int func(const ConnectedComponent &)): _min(1000000000), _max(-1), func(func) {
    for (const auto &component : forest) {
        int feature = func(component);
        _min = std::min(feature, _min);
        _max = std::max(feature, _max);
    }
}

int ExternalFeautureGetter::max() const {
    return _max;
}
int ExternalFeautureGetter::min() const {
    return _min;
}

int ExternalFeautureGetter::get_feature(const ConnectedComponent &component) const {
    return func(component);
    return black_area(component);
    return component.width() * component.height();
}

int feature_height(const ConnectedComponent &component) {
    return component.height();
}

int feature_width(const ConnectedComponent &component) {
    return component.width();
}

int gradient(const ConnectedComponent &component, const GrayImage &image) {
    int result(0);
    //std::cout << image.size() << '\n';
    const array<ipair, 4> directions = {ipair(-1,0), ipair(1,0), ipair(0,-1), ipair(0,1)};
    for (int i = 0; i < component.height(); ++i) {
        for (int j = 0; j < component.width(); ++j) {
            if (component.form[i][j]) {
                for (const auto& direction : directions) {
                    try {
                        int i0 = i + direction.first, j0 = j + direction.second;
                        if (!component.form.at(i0).at(j0)) {
                            /*std::cout << i0 << ' ' << j0 << '\n';
                            std::cout << image.size() << ' ' << image[i + component.top].size() << '\n';*/
                            result += image.at(i + component.top).at(j + component.left) - image.at(i0 + component.top).at(j0 + component.left);
                            /*std::cout << i0 << ' ' << j0 << '\n';*/
                        }
                    }
                    catch (std::out_of_range&) {
                    }
                }
            }
        }
    }
    return result;
}

GradientFeatureGetter::GradientFeatureGetter(const ConnectedComponentForest &forest): image(forest.image) {
    for (const auto &component : forest) {
        int feature = get_feature(component);
        _min = std::min(feature, _min);
        _max = std::max(feature, _max);
    }
}

int GradientFeatureGetter::max() const {
    return _max;
}
int GradientFeatureGetter::min() const {
    return _min;
}

int GradientFeatureGetter::get_feature(const ConnectedComponent &component) const {
    return -gradient(component, image);
}
