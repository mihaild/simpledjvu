#pragma once

#include <array>
#include <vector>
#include "constants.h"

using std::array;
using std::vector;
using std::pair;

typedef int int32;
typedef unsigned char byte;

typedef array<int, COLORS_COUNT> Hystogram;

struct LevelsDistribution {
    byte background;
    byte foreground;
};

typedef vector<vector<LevelsDistribution> > GlobalLevelsDistribution;

typedef pair<int, int> ipair;

typedef vector<vector<bool> > bitonal_image;

typedef vector<vector<byte> > GrayImage;

struct Point {
    int x, y;
    Point(int x, int y): x(x), y(y) {
    }
    struct Point& operator += (const struct Point &other) {
        x += other.x;
        y += other.y;
        return *this;
    }
    struct Point operator + (const struct Point &other) {
        Point res(*this);
        res += other;
        return res;
    }
    struct Point& operator -= (const struct Point &other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }
    struct Point operator - (const struct Point &other) {
        Point res(*this);
        res -= other;
        return res;
    }
};
