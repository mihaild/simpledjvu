#include <vector>

using std::vector;

#include "djvulibre.h"

const int CELL_SIZE = 20;

const double QUANTILE = 0.05;

/*
 * fails if small.size > result.size/scale
 */
void increase_image(const vector<vector<double> > &small, GBitmap &result, int scale);

void get_image_parts(const GBitmap &image, GBitmap &black_result, GBitmap &white_result, int cell_size = CELL_SIZE);
