#pragma once

#include <vector>
#include <unordered_map>

#include "connected_components.h"

using std::vector;

class QualifierInterface {
    public:
        virtual double quality(const ConnectedComponent &component) const = 0;
        virtual ~QualifierInterface() {
        }
};

class AbstractFeatureGetter {
    public:
        virtual double max() const = 0;
        virtual double min() const = 0;
        virtual double get_feature(const ConnectedComponent &component) const = 0;
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

class ExternalFeautureGetter: public AbstractFeatureGetter, public QualifierInterface {
    private:
        double _min, _max;
        double (*func)(const ConnectedComponent &);
    public:
        ExternalFeautureGetter(const ConnectedComponentForest &forest, double (*func)(const ConnectedComponent &));
        virtual double max() const;
        virtual double min() const;
        virtual double get_feature(const ConnectedComponent &component) const;
        virtual double quality(const ConnectedComponent &component) const;
};

int gradient(const ConnectedComponent &component, const GrayImage &image);
int border_length(const ConnectedComponent &component, int width, int height);

class GradientFeatureGetter: public AbstractFeatureGetter, public QualifierInterface {
    private:
        double _min, _max;
        const GrayImage &image;
    public:
        GradientFeatureGetter(const ConnectedComponentForest &forest);
        virtual double max() const;
        virtual double min() const;
        virtual double get_feature(const ConnectedComponent &component) const;
        virtual double quality(const ConnectedComponent &component) const;
};

class HystogramQualifier: public QualifierInterface {
    private:
        vector<double> hystogram;
        AbstractFeatureGetter *feature_getter;
    public:
        HystogramQualifier(AbstractFeatureGetter *feature_getter, const ConnectedComponentForest &forest); 
        virtual double quality(const ConnectedComponent &component) const;
};

double feature_height(const ConnectedComponent &component);
double feature_width(const ConnectedComponent &component);
