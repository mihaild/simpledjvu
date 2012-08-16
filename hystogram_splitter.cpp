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

/*
 * fails if small.size > result.size/scale
 */
void increase_image(const vector<vector<double> > &small, GBitmap &result, int scale) {
    int width = result.columns(), height = result.rows();

    //corners
    for (int i = 0; i < scale / 2; ++i) {
        for (int j = 0; j < scale / 2; ++j) {
            result[i][j] = static_cast<int>(small[0][0]);
            result[height - i - 1][j] = static_cast<int>(small.back()[0]);
            result[i][width - j - 1] = static_cast<int>(small[0].back());
            result[height - i - 1][width - j - 1] = static_cast<int>(small.back().back());
        }
    }

    //left and right borders
    for (int i = 0; i < height - scale; ++i) {
        {
            double top_val = small[i / scale][0];
            double bottom_val = small[i / scale + 1][0];
            int pos = i % scale;
            for (int j = 0; j < scale / 2; ++j) {
                result[i + scale / 2][j] = static_cast<int>((bottom_val * pos + top_val * (scale - pos)) / scale);
            }
        }
        {
            double top_val = small[i / scale].back();
            double bottom_val = small[i / scale + 1].back();
            int pos = i % scale;
            for (int j = width - scale / 2; j < width; ++j) {
                result[i + scale / 2][j] = static_cast<int>((bottom_val * pos + top_val * (scale - pos)) / scale);
            }
        }
    }

    //top and bottom borders
    for (int j = 0; j < width - scale; ++j) {
        {
            double left_val = small[0][j / scale];
            double right_val = small[0][j / scale + 1];
            std::swap(left_val, right_val); //WTF?!?!?! why does it work?
            int pos = j % scale;
            for (int i = 0; i < scale / 2; ++i) {
                result[i][j + scale / 2] = static_cast<int>((left_val * pos + right_val * (scale - pos)) / scale);
            }
        }
        {
            double left_val = small.back()[j / scale];
            double right_val = small.back()[j / scale + 1];
            std::swap(left_val, right_val);
            int pos = j % scale;
            for (int i = height - scale / 2; i < height; ++i) {
                result[i][j + scale / 2] = static_cast<int>((left_val * pos + right_val * (scale - pos)) / scale);
            }
        }
    }
    for (int i = 0; i < height - scale; ++i) {
        for (int j = 0; j < width - scale; ++j) {
            int vpos = i % scale, hpos = j % scale;
            /*
             * a_b
             * |/|
             * c-a
             *
             * f(x, y) = A*x + B*y + C
             * f(1, 0) = b
             * f(0,1) = c
             *
             * (1, 1) = (scale - 1, scale - 1)
             */
            double A, B, C;
            double a, b, c;
            b = small[i / scale][j / scale + 1];
            c = small[i / scale + 1][j / scale];
            if (vpos*(scale - 1) + hpos*(scale - 1) < (scale - 1)*(scale - 1)) {//top left triangle
                a = small[i / scale][j / scale];
                C = a;
                A = b - a;
                B = c - a;
            }
            else {//bottom right triangle
                a = small[i / scale + 1][j / scale + 1];
                C = c + b - a;
                A = a - c;
                B = a - b;
            }
            result[i + scale / 2][j + scale / 2] = static_cast<int>((A*hpos*(scale-1) + B*vpos*(scale-1) + C*(scale-1)*(scale-1)) / ((scale-1)*(scale-1)));
        }
    }
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

    black_result.init(v_cells * back_scale, h_cells * back_scale);
    black_result.set_grays(256);
    increase_image(black, black_result, back_scale);

    white_result.init(v_cells * back_scale, h_cells * back_scale);
    white_result.set_grays(256);
    increase_image(white, white_result, back_scale);
}
