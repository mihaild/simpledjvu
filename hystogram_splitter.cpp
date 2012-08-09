#include <cstdlib>
#include <cmath>

#include "pgm.h"
#include "hystograms.h"

const int CELL_SIZE = 20;

const int ROUNDS = 50;

const double QUANTILE = 0.05;

const int EPS_STEP = 5;

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

double get_val(double q, double dispersion, double avg) {
    return (1 - dispersion) * avg + dispersion * q;
}

GrayImage make_step(const GrayImage &q, const vector<vector<double> > &dispersions, const GrayImage &image) {
    GrayImage result(image);
    int width = image[0].size(), height = image.size();

    for (int i = 0; i < result.size(); ++i) {
        for (int j = 0; j < result[i].size(); ++j) {
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
            result[i][j] = get_val(q[i][j], dispersions[i][j], static_cast<double> (s) / n);
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

    GrayImage black_q(height / CELL_SIZE, vector<byte> (width / CELL_SIZE));
    GrayImage white_q(black_q);
    vector<vector<double> > dispersions(height / CELL_SIZE, vector<double> (width / CELL_SIZE));

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

    GrayImage black(black_q), white(white_q);

    for (int round = 0; round < ROUNDS; ++round) {
        black = make_step(black_q, dispersions, black);
        white = make_step(white_q, dispersions, white);
    }

    save_pgm(fopen(argv[2], "w"), black);
    save_pgm(fopen(argv[3], "w"), white);

    if (argc > 4) {
        save_pgm(fopen(argv[4], "w"), black_q);
        save_pgm(fopen(argv[5], "w"), white_q);
    }

    return 0;
}
