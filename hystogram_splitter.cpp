#include <cstdlib>
#include <cmath>
#include <algorithm>

#include "djvulibre.h"

#include "types.h"
#include "hystogram_splitter.h"

byte get_right_quantile(const Hystogram &hystogram, double level) {
    int need = std::accumulate(hystogram.begin(), hystogram.end(), 0) * level;
    int current_sum = 0;
    for (byte i = hystogram.size() - 1; i >= 0; --i) {
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

vector<vector<double> > make_step(const GBitmap &q, const vector<vector<double> > &image, bool up, double eps_step) {
    vector<vector<double> > result(image);
    int width = q.columns(), height = q.rows();
    int direction = up ? 1 : -1;

    for (int i = 0; i < result.size(); ++i) {
        for (int j = 0; j < result[i].size(); ++j) {
            if (direction*(q[i][j] - image[i][j]) > 0) {
                double n(0), s(0);
                if (i > 0) {
                    ++n;
                    s += image[i-1][j];
                }
                if (i < result.size() - 1) {
                    ++n;
                    s += image[i+1][j];
                }
                if (j > 0) {
                    ++n;
                    s += image[i][j-1];
                }
                if (j < result[i].size() - 1) {
                    ++n;
                    s += image[i][j+1];
                }
                double avg = s / n;
                if (up) {
                    result[i][j] = (avg + eps_step > q[i][j]) ? q[i][j] : (avg + eps_step);
                }
                else {
                    result[i][j] = (avg - eps_step < q[i][j]) ? q[i][j] : (avg - eps_step);
                }
            }
        }
    }
    return result;
}

void get_image_parts(const GBitmap &image, GBitmap &black_result, GBitmap &white_result, int cell_size, int back_scale) {
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

    double eps_step = 255.0 / std::max(v_cells, h_cells) / 10.0;
    int rounds = 255.0 / eps_step + 1;
    for (int round = 0; round < rounds; ++round) {
        black = make_step(black_q, black, false, eps_step);
        white = make_step(white_q, white, true, eps_step);
    }

    black_result.init(v_cells, h_cells);
    black_result.set_grays(256);
    white_result.init(v_cells, h_cells);
    white_result.set_grays(256);
    for (int i = 0; i < v_cells; ++i) {
        for (int j = 0; j < h_cells; ++j) {
            black_result[i][j] = static_cast<byte> (black[i][j]);
            white_result[i][j] = static_cast<byte> (white[i][j]);
        }
    }
    //increase_image(black, black_result, back_scale);

    //increase_image(white, white_result, back_scale);
}
