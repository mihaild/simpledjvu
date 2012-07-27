#include <algorithm>
#include <iostream>
#include <stdexcept>

#include "connected_components.h"
#include "quality.h"

using std::min;
using std::max;

HystogramQualifier::HystogramQualifier(AbstractFeatureGetter *feature_getter, const ConnectedComponentForest &forest): feature_getter(feature_getter) {
    for (const auto &component : forest) {
        hystogram.push_back(feature_getter->get_feature(component));
    }
    sort(hystogram.begin(), hystogram.end());
}

double HystogramQualifier::quality(const ConnectedComponent &component) const {
    double value = feature_getter->get_feature(component);
    double left_sum = static_cast<double> (std::lower_bound(hystogram.begin(), hystogram.end(), value) - hystogram.begin());
    double right_sum = static_cast<double> (hystogram.end() - std::upper_bound(hystogram.begin(), hystogram.end(), value));
    double exact = hystogram.size() - left_sum - right_sum;
    return min(left_sum, right_sum);
}

double black_area(const ConnectedComponent &component) {
    return component.width() * component.height();
    int res(0);
    for (const auto &i : component.form) {
        for (const auto &j : i) {
            res += j;
        }
    }
    return res;
}

ExternalFeautureGetter::ExternalFeautureGetter(const ConnectedComponentForest &forest, double func(const ConnectedComponent &)): _min(1000000000), _max(-1), func(func) {
    for (const auto &component : forest) {
        double feature = func(component);
        _min = std::min(feature, _min);
        _max = std::max(feature, _max);
    }
}

double ExternalFeautureGetter::max() const {
    return _max;
}
double ExternalFeautureGetter::min() const {
    return _min;
}

double ExternalFeautureGetter::get_feature(const ConnectedComponent &component) const {
    return func(component);
    return black_area(component);
    return component.width() * component.height();
}

double feature_height(const ConnectedComponent &component) {
    return static_cast<double>(component.height());
}

double feature_width(const ConnectedComponent &component) {
    return static_cast<double>(component.width());
}

int gradient(const ConnectedComponent &component, const GrayImage &image) {
    int result(0);
    const array<ipair, 4> directions = {ipair(-1,0), ipair(1,0), ipair(0,-1), ipair(0,1)};
    for (int i = 0; i < component.height(); ++i) {
        for (int j = 0; j < component.width(); ++j) {
            if (component.form[i][j]) {
                for (const auto& direction : directions) {
                    try {
                        int i0 = i + direction.first, j0 = j + direction.second;
                        if (!component.form.at(i0).at(j0)) {
                            result += image.at(i + component.top).at(j + component.left) - image.at(i0 + component.top).at(j0 + component.left);
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
int border_length(const ConnectedComponent &component, int width, int height) {
    int result(0);
    const array<ipair, 4> directions = {ipair(-1,0), ipair(1,0), ipair(0,-1), ipair(0,1)};
    for (int i = 0; i < component.height(); ++i) {
        for (int j = 0; j < component.width(); ++j) {
            if (component.form[i][j]) {
                for (const auto& direction : directions) {
                    try {
                        int i0 = i + direction.first, j0 = j + direction.second;
                        if (!component.form.at(i0).at(j0)) {
                            ++result;
                        }
                    }
                    catch (std::out_of_range&) {
                    }
                }
            }
        }
    }
    if (component.left == 0) {
        for (int j = 0; j < component.height(); ++j) {
            result += component.form[j][0];
        }
    }
    if (component.right == width - 1) {
        for (int j = 0; j < component.height(); ++j) {
            result += component.form[j].back();
        }
    }
    if (component.top == 0) {
        for (int j = 0; j < component.width(); ++j) {
            result += component.form[0][j];
        }
    }
    if (component.top == height - 1) {
        for (int j = 0; j < component.width(); ++j) {
            result += component.form.back()[j];
        }
    }
    //return 1.0;
    return result;
}

GradientFeatureGetter::GradientFeatureGetter(const ConnectedComponentForest &forest): image(forest.image), _min(1000000000), _max(-1) {
    for (const auto &component : forest) {
        double feature = get_feature(component);
        _min = std::min(feature, _min);
        _max = std::max(feature, _max);
    }
}

double GradientFeatureGetter::max() const {
    return _max;
}
double GradientFeatureGetter::min() const {
    return _min;
}

double GradientFeatureGetter::get_feature(const ConnectedComponent &component) const {
    return -static_cast<double>(gradient(component, image)) / border_length(component, image[0].size(), image.size());
}
