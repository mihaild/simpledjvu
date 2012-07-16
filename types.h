#pragma once

#include <array>
#include <vector>
#include "constants.h"

using std::array;
using std::vector;

typedef int int32;
typedef unsigned char byte;

typedef array<int, COLORS_COUNT> Hystogram;
typedef array<int, QUANT> LevelsDistribution;
typedef vector<vector<LevelsDistribution> > GlobalLevelsDistribution;
