#pragma once

#include "types.h"
#include "constants.h"

vector<vector<Hystogram> > get_rectangle_hystograms(byte *pixels, int row_size, int rows_count);
Hystogram get_local_hystogram(const vector<vector<Hystogram> > &rectangles_hystogram, int radius, int x, int y);
byte get_right_quantile(const Hystogram &hystogram, double level);
byte get_left_quantile(const Hystogram &hystogram, double level);
