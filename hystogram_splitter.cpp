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

    GrayImage big(image);
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            big[i][j] = black[i / CELL_SIZE][j / CELL_SIZE];
        }
    }
    save_pgm(fopen(argv[2], "w"), big);

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            big[i][j] = white[i / CELL_SIZE][j / CELL_SIZE];
        }
    }
    save_pgm(fopen(argv[3], "w"), white);

    if (argc > 4) {
        save_pgm(fopen(argv[4], "w"), black_q);
        save_pgm(fopen(argv[5], "w"), white_q);
    }

    return 0;
}
