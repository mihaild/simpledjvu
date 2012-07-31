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

double ExternalFeautureGetter::quality(const ConnectedComponent &component) const {
    return get_feature(component);
}

double feature_height(const ConnectedComponent &component) {
    return static_cast<double>(component.height());
}

double feature_width(const ConnectedComponent &component) {
    return static_cast<double>(component.width());
}

int border_sum(int func(int, int), const ConnectedComponent &component, const GrayImage &image) {
    int result(0);
    for (int i = 1; i < component.height(); ++i) {
        for (int j = 0; j < component.width(); ++j) {
            if (component.form[i][j] != component.form[i-1][j]) {
                if (component.form[i][j]) {
                    result += func(image[i + component.top][j + component.left], image[i + component.top - 1][j + component.left]);
                }
                else {
                    result += func(image[i + component.top - 1][j + component.left], image[i + component.top][j + component.left]);
                }
            }
        }
    }
    for (int i = 0; i < component.height(); ++i) {
        for (int j = 1; j < component.width(); ++j) {
            if (component.form[i][j] != component.form[i][j-1]) {
                if (component.form[i][j]) {
                    result += func(image[i + component.top][j + component.left], image[i + component.top][j + component.left - 1]);
                }
                else {
                    result += func(image[i + component.top][j + component.left - 1], image[i + component.top][j + component.left]);
                }
            }
        }
    }
    return result;
}

int gradient(const ConnectedComponent &component, const GrayImage &image) {
    return border_sum([](int in, int out) { return in-out; }, component, image);
}

int border_length(const ConnectedComponent &component, const GrayImage &image) {
    int result = border_sum([](int in, int out) { return 1; }, component, image);
    int height = image.size(), width = image[0].size();
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
    return -static_cast<double>(gradient(component, image)) / border_length(component, image);
}

double GradientFeatureGetter::quality(const ConnectedComponent &component) const {
    return get_feature(component);
}
