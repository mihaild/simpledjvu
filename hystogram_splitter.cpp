#include <iostream>
#include <cstdlib>
#include <cmath>

#include "hystograms.h"

#include "DjVuGlobal.h"
#include "GException.h"
#include "GSmartPointer.h"
#include "GContainer.h"
#include "ByteStream.h"
#include "IFFByteStream.h"
#include "GRect.h"
#include "GBitmap.h"
#include "JB2Image.h"
#include "DjVuInfo.h"
#include "GOS.h"
#include "GURL.h"
#include "DjVuMessage.h"

const int CELL_SIZE = 20;

const double QUANTILE = 0.05;

double EPS_STEP;
int ROUNDS;

vector<vector<double> > make_step(const GBitmap &q, const vector<vector<double> > &image, bool up) {
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

void increase_image(const vector<vector<double> > &small, GBitmap &result, int scale = CELL_SIZE) {
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

int main(int argc, char *argv[]) {
    GP<ByteStream> data = ByteStream::create(GURL::Filename::UTF8(argv[1]), "rb");
    GP<GBitmap> gsource = GBitmap::create(*data);
    GBitmap &source = *gsource;

    int32 width, height;
    width = source.columns();
    height = source.rows();

    GrayImage image(height, vector<byte> (width));
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            image[i][j] = source[i][j];
        }
    }

    int32 v_cells = height / CELL_SIZE + (height % CELL_SIZE ? 1 : 0);
    int32 h_cells = width / CELL_SIZE + (width % CELL_SIZE ? 1 : 0);

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
            for (int k = 0; k < CELL_SIZE && i*CELL_SIZE + k < height; ++k) {
                for (int l = 0; l < CELL_SIZE && j*CELL_SIZE + l < width; ++l) {
                    ++hystogram[image[i*CELL_SIZE + k][j*CELL_SIZE + l]];
                }
            }
            black_q[i][j] = get_right_quantile(hystogram, QUANTILE);
            white_q[i][j] = get_left_quantile(hystogram, QUANTILE);
        }
    }

    vector<vector<double> > black(v_cells, vector<double> (h_cells, 255));
    vector<vector<double> > white(v_cells, vector<double> (h_cells, 0));

    EPS_STEP = 255.0 / std::max(v_cells, h_cells) / 10.0;
    ROUNDS = 255.0 / EPS_STEP + 1;
    std::cerr << "ROUNDS: " << ROUNDS << ", EPS_STEP: " << EPS_STEP << '\n';
    for (int round = 0; round < ROUNDS; ++round) {
        black = make_step(black_q, black, false);
        white = make_step(white_q, white, true);
    }

    GP<GBitmap> gblack_original_size = GBitmap::create(height, width);
    GBitmap &black_original_size = *gblack_original_size;
    black_original_size.set_grays(256);
    increase_image(black, black_original_size);
    black_original_size.save_pgm(*ByteStream::create(GURL::Filename::UTF8(argv[2]), "wb"));

    GP<GBitmap> gwhite_original_size = GBitmap::create(height, width);
    GBitmap &white_original_size = *gwhite_original_size;
    white_original_size.set_grays(256);
    increase_image(white, white_original_size);
    white_original_size.save_pgm(*ByteStream::create(GURL::Filename::UTF8(argv[3]), "wb"));

    return 0;
}
