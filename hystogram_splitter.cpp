/*
 * Simpledjvu-0.1
 * Based on djvulibre (http://djvu.sourceforge.net/)
 * Copyright 2012, Mikhail Dektyarev <mihail.dektyarow@gmail.com>
 *
 * This file is part of Simpledjvu.
 * 
 * Simpledjvu is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Foobar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Simpledjvu.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <djvulibre.h>

#include <types.h>
#include <hystogram_splitter.h>

#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <vector>
#include <array>

using std::min;
using std::max;
using std::array;


typedef array<int, COLORS_COUNT> Hystogram;

byte get_right_quantile(const Hystogram &hystogram, double level) {
    int need = std::accumulate(hystogram.begin(), hystogram.end(), 0) * level;
    int current_sum = 0;
    for (int i = hystogram.size() - 1; i >= 0; --i) {
        current_sum += hystogram[i];
        if (current_sum >= need) {
            return i;
        }
    }
    return 0;
}

byte get_left_quantile(const Hystogram &hystogram, double level) {
    return get_right_quantile(hystogram, 1.0 - level);
}

/*
 * get = min and eps < 0, or get = max and eps > 0
 * otherwise it destroys the Earth
 */
template<const double& (*get)(const double &, const double &)>
  void get_values(const GBitmap &q, const double eps_step, vector<vector<double> > *result) {
    vector<vector<double> > row_val(q.rows(), vector<double> (q.columns()));
    {
        vector<vector<double> > left_val(q.rows(), vector<double> (q.columns())), right_val(q.rows(), vector<double> (q.columns()));
        for (int i = 0; i < q.rows(); ++i) {
            left_val[i][0] = q[i][0];
            for (int j = 1; j < q.columns(); ++j) {
                left_val[i][j] = get(left_val[i][j-1] + eps_step, static_cast<double>(q[i][j]));
            }
            right_val[i].back() = q[i][q.columns() - 1];
            for (int j = q.columns() - 2; j >= 0; --j) {
                right_val[i][j] = get(right_val[i][j+1] + eps_step, static_cast<double>(q[i][j]));
            }
            for (int j = 0; j < q.columns(); ++j) {
                row_val[i][j] = get(left_val[i][j], right_val[i][j]);
            }
        }
    }
    vector<vector<double> > up_val(q.rows(), vector<double> (q.columns())), down_val(q.rows(), vector<double> (q.columns()));
    for (int j = 0; j < q.columns(); ++j) {
        up_val[0][j] = row_val[0][j];
        for (int i = 1; i < q.rows(); ++i) {
            up_val[i][j] = get(row_val[i][j], up_val[i-1][j] + eps_step);
        }
        down_val.back()[j] = row_val.back()[j];
        for (int i = q.rows() - 2; i >= 0; --i) {
            down_val[i][j] = get(row_val[i][j], up_val[i+1][j] + eps_step);
        }
    }
    for (int i = 0; i < q.rows(); ++i) {
        for (int j = 0; j < q.columns(); ++j) {
            (*result)[i][j] = get(up_val[i][j], down_val[i][j]);
        }
    }
}

/*
 * @todo: different scales for black and white
 */
void get_image_parts(const GBitmap &image, GBitmap &black_result, GBitmap &white_result, int cell_size) {
    int width = image.columns(), height = image.rows();
    int v_cells = height / cell_size + (height % cell_size ? 1 : 0), h_cells = width / cell_size + (width % cell_size ? 1 : 0);
    GP<GBitmap> gblack_q = GBitmap::create(v_cells, h_cells);
    GP<GBitmap> gwhite_q = GBitmap::create(v_cells, h_cells);
    GBitmap &black_q = *gblack_q;
    GBitmap &white_q = *gwhite_q;

    for (int i = 0; i < v_cells; ++i) {
        for (int j = 0; j < h_cells; ++j) {
            Hystogram hystogram;
            for (auto &k : hystogram) {
                k = 0;
            }
            for (int k = 0; k < cell_size && i*cell_size + k < height; ++k) {
                for (int l = 0; l < cell_size && j*cell_size + l < width; ++l) {
                    ++hystogram[image[i*cell_size + k][j*cell_size + l]];
                }
            }
            black_q[i][j] = get_right_quantile(hystogram, QUANTILE);
            white_q[i][j] = get_left_quantile(hystogram, QUANTILE);
        }
    }

    vector<vector<double> > black(v_cells, vector<double> (h_cells, 255));
    vector<vector<double> > white(v_cells, vector<double> (h_cells, 0));

    double eps_step = 255.0 / std::max(v_cells, h_cells) / 2.0;

    get_values<max> (black_q, -eps_step, &black);
    get_values<min> (white_q, eps_step, &white);

    black_result.init(v_cells, h_cells);
    black_result.set_grays(256);
    white_result.init(v_cells, h_cells);
    white_result.set_grays(256);
    for (int i = 0; i < v_cells; ++i) {
        for (int j = 0; j < h_cells; ++j) {
            black_result[i][j] = static_cast<byte> (black[i][j]);
            white_result[i][j] = static_cast<byte> (white[i][j]);
            if (black_result[i][j] - white_result[i][j] < MIN_COLORS_DIFF) {
                if (black_result[i][j] > 255 - MIN_COLORS_DIFF) {
                    white_result[i][j] -= 255 - black_result[i][j];
                    black_result[i][j] = 255;
                }
                else {
                    black_result[i][j] += MIN_COLORS_DIFF;
                }
            }
        }
    }
}
