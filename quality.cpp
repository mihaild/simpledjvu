#include <algorithm>
#include <iostream>
#include <stdexcept>

#include "types.h"
#include "connected_components.h"
#include "quality.h"

#include <cmath>

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

GradientFeatureGetter::GradientFeatureGetter(const ConnectedComponentForest &forest): image(forest.image), _min(1.0), _max(1.0) {
    double real_max(-1.0), real_min(1e99);
    for (const auto &component : forest) {
        double feature = get_feature(component);
        real_min = std::min(feature, real_min);
        real_max = std::max(feature, real_max);
    }
    _min = real_min;
    _max = real_max;
}

double GradientFeatureGetter::max() const {
    return _max;
}
double GradientFeatureGetter::min() const {
    return _min;
}

double GradientFeatureGetter::get_feature(const ConnectedComponent &component) const {
    return -static_cast<double>(gradient(component, image)) / border_length(component, image) / max();
}

double GradientFeatureGetter::quality(const ConnectedComponent &component) const {
    return get_feature(component);
}

AvgDistanceFeatureGetter::AvgDistanceFeatureGetter(const ConnectedComponentForest &forest): forest(forest) {
}

array<double, 4> parts(const ConnectedComponent &component) {
    Point c = component.mass_center();
    array<double, 4> result = {0,0,0,0};
    for (int i = 0; i < c.y; ++i) {
        for (int j = 0; j < c.x; ++j) {
            result[0] += component.form[i][j];
        }
        for (int j = c.x; j < component.width(); ++j) {
            result[1] += component.form[i][j];
        }
    }
    for (int i = c.y; i < component.height(); ++i) {
        for (int j = 0; j < c.x; ++j) {
            result[2] += component.form[i][j];
        }
        for (int j = c.x; j < component.width(); ++j) {
            result[3] += component.form[i][j];
        }
    }
    double sum = result[0]+result[1]+result[2]+result[3];
    for (auto &i : result) {
        i /= sum;
    }
    return result;
}

double AvgDistanceFeatureGetter::distance(const ConnectedComponent &a, const ConnectedComponent &b) const {
    if (max(a.width(), b.width()) > 2*min(a.width(), b.width()) || max(a.height(), b.height()) > 2*min(a.height(), b.height())) {//difference in sizes is too big
        return 0.0;
    }
    //array<double, 4> parts_a = parts(a), parts_b = parts(b);
    //double result(0);
    //for (int i = 0; i < 4; ++i) {
        //result += fabs(parts_a[i] - parts_b[i]);
    //}
    //return result;
    //int overlap(0), area(0);
    //for (int i = 0; i < a.height() && i < b.height(); ++i) {
        //for (int j = 0; j < a.width() && j < b.width(); ++j) {
            //++area;
            //overlap += a.form[i][j] == b.form[i][j];
        //}
    //}
    
    Point ca = a.mass_center(), cb = b.mass_center();
    //consider a[0][0] as origin
    Point b_left_top(ca - cb), b_right_bottom(ca - cb + Point(b.width(), b.height()));
    int overlap(0);
    int area(0);
    for (int i = max(0, b_left_top.y); i < min(a.height(), b_right_bottom.y); ++i) {
        for (int j = max(0, b_left_top.x); j < min(a.width(), b_right_bottom.x); ++j) {
            overlap += a.form[i][j] == b.form[i - b_left_top.y][j - b_left_top.x];
            ++area;
        }
    }
    return static_cast<double> (overlap) / area;
    //return 1.0;
}

double AvgDistanceFeatureGetter::quality(const ConnectedComponent &component) const {
    double result = 0.0;
    for (const auto &i : forest) {
        double d = distance(component, i);
        if (d > 0.9) {
            result += distance(component, i);
        }
    }
    return result;
}

double VarianceFeatureGetter::dispersion(const ConnectedComponent &component) const {
    int n(0);
    double x(0.0), x_sq(0.0);
    for (int i = 0; i < component.height(); ++i) {
        for (int j = 0; j < component.width(); ++j) {
            if (component.form[i][j]) {
                ++n;
                double val = image[component.top + i][component.left + j] / 255.0;
                x += val;
                x_sq += val*val;
            }
        }
    }
    double disp = (x_sq / n - pow((x / n), 2));;
    return disp;
}

VarianceFeatureGetter::VarianceFeatureGetter(const ConnectedComponentForest &forest): image(forest.image), _min(9999), _max(-1) {
    for (const auto &i : forest) {
        double d = dispersion(i);
        _min = min(d, _min);
        _max = max(d, _max);
    }
}

double VarianceFeatureGetter::quality(const ConnectedComponent &component) const {
    return 1 - ((dispersion(component) - _min) / (_max - _min)) - 0.5;
}
