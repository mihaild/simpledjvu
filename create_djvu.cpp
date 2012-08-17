#include <iostream>

using std::cout;

#include "djvulibre.h"

#include "hystogram_splitter.h"
#include "normalize.h"
#include "select_threshold_level.h"

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

/*
 * it should be const GBitmap, but for unknown reason GBitmap::save_pgm is not declared as const
 */
void save(GBitmap &image, const char *fname, bool pgm = true) {
    if (pgm) {
        image.save_pgm(*ByteStream::create(GURL::Filename::UTF8(fname), "wb"));
    }
    else {
        image.save_pbm(*ByteStream::create(GURL::Filename::UTF8(fname), "wb"));
    }
}

GP<GBitmap> get_norm_image(const GBitmap &image) {
    GP<GBitmap> gresult = GBitmap::create();

    GP<GBitmap> gblack_small = GBitmap::create();
    GBitmap &black_small = *gblack_small;
    GP<GBitmap> gwhite_small = GBitmap::create();
    GBitmap &white_small = *gwhite_small;

    get_image_parts(image, black_small, white_small, CELL_SIZE, CELL_SIZE);
    
    GP<GBitmap> gblack = GBitmap::create(image.rows(), image.columns());
    GBitmap &black = *gblack;
    GP<GBitmap> gwhite = GBitmap::create(image.rows(), image.columns());
    GBitmap &white = *gwhite;

    rescale_bitmap(black_small, black);
    rescale_bitmap(white_small, white);

    /*save(black, "black.pgm");
    save(white, "white.pgm");*/

    normalize(image, black, white, *gresult);

    return gresult;
}

int main(int argc, char *argv[]) {
    GP<GBitmap> gsource = GBitmap::create(*ByteStream::create(GURL::Filename::UTF8(argv[1]), "rb"));
    GBitmap &image = *gsource;

    GP<GBitmap> gnormalized = GBitmap::create(image.rows() * 2, image.columns() * 2);
    rescale_bitmap(*get_norm_image(image), *gnormalized);
    gnormalized->binarize_grays(get_threshold_level(*gnormalized));
    save(*gnormalized, argv[2], false);
    /*GBitmap &result = *gresult;
    result.save_pgm(*ByteStream::create(GURL::Filename::UTF8(argv[2]), "wb"));*/


    return 0;
}
