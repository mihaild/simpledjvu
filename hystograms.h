#include "types.h"
#include "constants.h"
#include "pgm.h"

vector<vector<Hystogram> > get_rectangle_hystograms(byte *pixels, int row_size, int rows_count);
Hystogram get_local_hystogram(const vector<vector<Hystogram> > &rectangles_hystogram, int radius, int x, int y);