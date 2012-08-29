#include <stdlib.h>
#include <stdio.h>

#include "djvulibre.h"

#include "normalize.h"
#include "types.h"
#include "hystogram_splitter.h"
#include <iostream>


/*
 * hope, that black > white
 * else it can show strange picture, crash, or destroy the Earth
 */
byte canonize_level(const byte raw, const byte black, const byte white) {
    if (raw >= black) {
        return CANONICAL_BLACK_LEVEL;
    }
    if (raw <= white) {
        return CANONICAL_WHITE_LEVEL;
    }
    return (raw - white) * (CANONICAL_BLACK_LEVEL - CANONICAL_WHITE_LEVEL) / (black - white);
}

void normalize_parts(const GBitmap &image, const GBitmap &black, const GBitmap &white, GBitmap &result) {
    int width = image.columns(), height = image.rows();
    result.init(height, width);
    result.set_grays(256);
    int32 i, j;
    int32 target;
    for (i = 0; i < height; ++i) {
        for (j = 0; j < width; ++j) {
            result[i][j] = canonize_level(image[i][j], black[i][j], white[i][j]);
        }
    }
}

/*
 * example from http://www.djvuzone.org/open/doc/GBitmapScaler.html
 */
void rescale_bitmap(const GBitmap &in, GBitmap &out)
{
    int w = in.columns();       // Get input width
    int h = in.rows();          // Get output width
    int nw = out.columns();
    int nh = out.rows();
    GP<GBitmapScaler> gscaler = GBitmapScaler::create(w,h,nw,nh);  // Creates bitmap scaler
    GRect desired(0,0,nw,nh);   // Desired output = complete bitmap
    GRect provided(0,0,w,h);    // Provided input = complete bitmap
    gscaler->scale(provided, in, desired, out);  // Rescale
}

namespace {
    void save(GBitmap &image, const char *fname, bool pgm = true) {
        if (pgm) {
            image.save_pgm(*ByteStream::create(GURL::Filename::UTF8(fname), "wb"));
        }
        else {
            image.save_pbm(*ByteStream::create(GURL::Filename::UTF8(fname), "wb"));
        }
    }
}

GP<GBitmap> get_norm_image(const GBitmap &image) {
    GP<GBitmap> current = GBitmap::create(image);
    GP<GBitmap> next = GBitmap::create();
    for (int i = 0; i < 200; ++i) {
        std::cout << i << '\n';
        GP<GBitmap> gblack_small = GBitmap::create();
        GBitmap &black_small = *gblack_small;
        GP<GBitmap> gwhite_small = GBitmap::create();
        GBitmap &white_small = *gwhite_small;

        get_image_parts(*current, black_small, white_small, CELL_SIZE);

        save(black_small, "black.pgm");
        save(white_small, "white.pgm");

        GP<GBitmap> gblack = GBitmap::create(image.rows(), image.columns());
        GBitmap &black = *gblack;
        GP<GBitmap> gwhite = GBitmap::create(image.rows(), image.columns());
        GBitmap &white = *gwhite;

        rescale_bitmap(black_small, black);
        rescale_bitmap(white_small, white);

        normalize_parts(*current, black, white, *next);

        std::swap(current, next);
    }

    return current;
}
