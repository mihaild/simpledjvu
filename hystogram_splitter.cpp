#include <iostream>
#include <cstdlib>
#include <cmath>

#include "pgm.h"
#include "hystograms.h"

const int CELL_SIZE = 20;

const int ROUNDS = 1000;

const double QUANTILE = 0.05;

const byte EPS_STEP = 1;

double dispersion(const Hystogram &hystogram) {
    double sum(0.0), sum_sqr(0.0);
    int elements(0);
    for (int i = 0; i < COLORS_COUNT; ++i) {
        sum += i*hystogram[i];
        sum_sqr += i*i*hystogram[i];
        elements += hystogram[i];
    }
    if (!elements) {
        return 0;
    }
    return sqrt(sum_sqr/elements - sum*sum/(elements*elements)) / (COLORS_COUNT / 2);
}

GrayImage make_step(const GrayImage &q, const vector<vector<double> > &dispersions, const GrayImage &image, bool up) {
    GrayImage result(image);
    int width = image[0].size(), height = image.size();
    int direction = up ? 1 : -1;

    for (int i = 0; i < result.size(); ++i) {
        for (int j = 0; j < result[i].size(); ++j) {
            if (direction*(q[i][j] - image[i][j]) > 0) {
                int n(0), s(0);
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
                    result[i][j] = (avg + EPS_STEP > q[i][j]) ? q[i][j] : (avg + EPS_STEP);
                }
                else {
                    result[i][j] = (avg - EPS_STEP < q[i][j]) ? q[i][j] : (avg - EPS_STEP);
                }
            }
        }
    }
    return result;
}

GrayImage increase_image(const GrayImage &small, int width, int height) {
    GrayImage result(height, vector<byte> (width));
    //int vscale(height / small.size() + (height % small.size() == 0 ? 0 : 1)), hscale(width / small[0].size() + (width % small[0].size() == 0 ? 0 : 1));
    int vscale(CELL_SIZE), hscale(CELL_SIZE);

    //corners
    for (int i = 0; i < vscale / 2; ++i) {
        for (int j = 0; j < hscale / 2; ++j) {
            result[i][j] = small[0][0];
            result[height - i - 1][j] = small.back()[0];
            result[i][width - j - 1] = small[0].back();
            result[height - i - 1][width - j - 1] = small.back().back();
        }
    }

    //left and right borders
    for (int i = 0; i < height - vscale; ++i) {
        {
            int top_val = small[i / vscale][0];
            int bottom_val = small[i / vscale + 1][0];
            int pos = i % vscale;
            for (int j = 0; j < hscale / 2; ++j) {
                result[i + vscale / 2][j] = (bottom_val * pos + top_val * (vscale - pos)) / vscale;
            }
        }
        {
            int top_val = small[i / vscale].back();
            int bottom_val = small[i / vscale + 1].back();
            int pos = i % vscale;
            for (int j = width - hscale / 2; j < width; ++j) {
                result[i + vscale / 2][j] = (bottom_val * pos + top_val * (vscale - pos)) / vscale;
            }
        }
    }

    //top and bottom borders
    for (int j = 0; j < width - hscale; ++j) {
        {
            int left_val = small[0][j / hscale];
            int right_val = small[0][j / hscale + 1];
            std::swap(left_val, right_val); //WTF?!?!?! why it works?
            int pos = j % hscale;
            for (int i = 0; i < vscale / 2; ++i) {
                result[i][j + hscale / 2] = (left_val * pos + right_val * (hscale - pos)) / hscale;
            }
        }
        {
            int left_val = small.back()[j / hscale];
            int right_val = small.back()[j / hscale + 1];
            std::swap(left_val, right_val);
            int pos = j % hscale;
            for (int i = height - vscale / 2; i < height; ++i) {
                result[i][j + hscale / 2] = (left_val * pos + right_val * (hscale - pos)) / hscale;
            }
        }
    }
    for (int i = 0; i < height - vscale; ++i) {
        for (int j = 0; j < width - hscale; ++j) {
            int vpos = i % vscale, hpos = j % hscale;
            /*
             * a_b
             * |/|
             * c-a
             *
             * f(x, y) = A*x + B*y + C
             * f(1, 0) = b
             * f(0,1) = c
             *
             * (1, 1) = (hscale - 1, vscale - 1)
             */
            int A, B, C;
            int a, b, c;
            b = small[i / vscale][j / hscale + 1];
            c = small[i / vscale + 1][j / hscale];
            if (vpos*(hscale - 1) + hpos*(vscale - 1) < (hscale - 1)*(vscale - 1)) {//top left triangle
                a = small[i / vscale][j / hscale];
                C = a;
                A = b - a;
                B = c - a;
            }
            else {//bottom right triangle
                a = small[i / vscale + 1][j / vscale + 1];
                C = c + b - a;
                A = a - c;
                B = a - b;
            }
            result[i + vscale / 2][j + hscale / 2] = (A*hpos*(vscale-1) + B*vpos*(hscale-1) + C*(vscale-1)*(hscale-1)) / ((vscale-1)*(hscale-1));
        }
    }
    return result;
}

int main(int argc, char *argv[]) {
    FILE *data = fopen(argv[1], "r");
    byte *pixels;
    int32 width, height, row_size, rows_count;

    load_pgm(data, &width, &height, &row_size, &rows_count, &pixels, 0);

    GrayImage image = c_array_to_vector(pixels, width, height);

    int32 v_cells = height / CELL_SIZE + (height % CELL_SIZE ? 1 : 0);
    int32 h_cells = width / CELL_SIZE + (width % CELL_SIZE ? 1 : 0);

    GrayImage black_q(v_cells, vector<byte> (h_cells));
    GrayImage white_q(black_q);
    vector<vector<double> > dispersions(v_cells, vector<double> (h_cells));

    for (int i = 0; i < black_q.size(); ++i) {
        for (int j = 0; j < black_q[i].size(); ++j) {
            Hystogram hystogram;
            for (auto &k : hystogram) {
                k = 0;
            }
            for (int k = 0; k < CELL_SIZE && i*CELL_SIZE + k < height; ++k) {
                for (int l = 0; l < CELL_SIZE && j*CELL_SIZE + l < width; ++l) {
                    ++hystogram[image[i*CELL_SIZE + k][j*CELL_SIZE + l]];
                }
            }
            black_q[i][j] = get_left_quantile(hystogram, QUANTILE);
            white_q[i][j] = get_right_quantile(hystogram, QUANTILE);
            dispersions[i][j] = dispersion(hystogram);
        }
    }

    GrayImage black(v_cells, vector<byte> (h_cells, 0));
    GrayImage white(v_cells, vector<byte> (h_cells, 255));

    for (int round = 0; round < ROUNDS; ++round) {
        black = make_step(black_q, dispersions, black, true);
        white = make_step(white_q, dispersions, white, false);
    }

    save_pgm(fopen(argv[2], "w"), increase_image(black, width, height));

    save_pgm(fopen(argv[3], "w"), increase_image(white, width, height));

    if (argc > 4) {
        save_pgm(fopen(argv[4], "w"), black_q);
        save_pgm(fopen(argv[5], "w"), white_q);
    }

    return 0;
}
