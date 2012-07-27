#pragma once

#include <vector>

#include "connected_components.h"

using std::vector;

class AbstractFeatureGetter {
    public:
        virtual int max() const = 0;
        virtual int min() const = 0;
        virtual int get_feature(const ConnectedComponent &component) const = 0;
        virtual ~AbstractFeatureGetter() {
        }
};
/*
class AreaFeatureGetter: public AbstractFeatureGetter {
    private:
        int _min, _max;
    public:
        AreaFeatureGetter(const ConnectedComponentForest &forest);
        virtual int max();
        virtual int min();
        virtual int get_feature(const ConnectedComponent &component) const;
};*/

class ExternalFeautureGetter: public AbstractFeatureGetter {
    private:
        int _min, _max;
        int (*func)(const ConnectedComponent &);
    public:
        ExternalFeautureGetter(const ConnectedComponentForest &forest, int (*func)(const ConnectedComponent &));
        virtual int max() const;
        virtual int min() const;
        virtual int get_feature(const ConnectedComponent &component) const;
};

int gradient(const ConnectedComponent &component, const GrayImage &image);

class GradientFeatureGetter: public AbstractFeatureGetter {
    private:
        int _min, _max;
        const GrayImage &image;
    public:
        GradientFeatureGetter(const ConnectedComponentForest &forest);
        virtual int max() const;
        virtual int min() const;
        virtual int get_feature(const ConnectedComponent &component) const;
};

class HystogramQualifier {
    private:
        vector<int> hystogram;
        vector<int> hystogram_sums;
        AbstractFeatureGetter *feature_getter;
    public:
        HystogramQualifier(AbstractFeatureGetter *feature_getter, const ConnectedComponentForest &forest); 
        double quality(const ConnectedComponent &component) const;
};

int feature_height(const ConnectedComponent &component);
int feature_width(const ConnectedComponent &component);
