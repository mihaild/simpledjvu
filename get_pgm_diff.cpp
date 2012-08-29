#include <iostream>
#include <cmath>
#include <vector>

using std::vector;

#include "djvulibre.h"

vector<vector<int> > get_image_diff(const GBitmap &image1, const GBitmap &image2) {
    vector<vector<int> > result(image1.columns(), vector<int> (image1.rows()));

    for (int i = 0; i < result.size(); ++i) {
        for (int j = 0; j < result[i].size(); ++j) {
            result[i][j] = static_cast<int>(image1[i][j])  - static_cast<int>(image2[i][j]);
        }
    }

    return result;
}

double Lp_norm(const vector<vector<int> > &data, double p = 1.0) {
    double result(0.0);

    for (int i = 0; i < data.size(); ++i) {
        for (int j = 0; j < data[i].size(); ++j) {
            result += pow(fabs(data[i][j]), p);
        }
    }

    return pow(result, 1.0 / p);
}

double Lp_diff(const GBitmap &image1, const GBitmap &image2, double p = 2.0) {
    vector<vector<int> > diff = get_image_diff(image1, image2);

    return Lp_norm(diff, p);
}

int main(int argc, char *argv[]) {
    GP<GBitmap> gimage1 = GBitmap::create(*ByteStream::create(GURL::Filename::UTF8(argv[1]), "rb"));
    GP<GBitmap> gimage2 = GBitmap::create(*ByteStream::create(GURL::Filename::UTF8(argv[2]), "rb"));

    if (gimage1->columns() != gimage2->columns() || gimage1->rows() != gimage2->rows()) {
        std::cerr << "Image sizes don't match\n";
        return 1;
    }

    //std::cout << "Area: " << gimage1->rows() * gimage1->columns() << '\n';

    auto diff = get_image_diff(*gimage1, *gimage2);

    std::cout << /*"L1: " <<*/ static_cast<long long int>(Lp_norm(diff, 1.0)) /*<< "; L2:"*/ << "  " << Lp_norm(diff, 2.0) << '\n';
    return 0;
}
