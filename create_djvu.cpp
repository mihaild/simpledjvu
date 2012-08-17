#include <iostream>

using std::cout;

#include "djvulibre.h"

#include "hystogram_splitter.h"
#include "normalize.h"
#include "select_threshold_level.h"

/*
 * it should be const GBitmap, but for unknown reason GBitmap::save_pgm is not declared as const
 */
void save(GBitmap &image, const char *fname) {
    image.save_pgm(*ByteStream::create(GURL::Filename::UTF8(fname), "wb"));
}

int main(int argc, char *argv[]) {
    GP<GBitmap> gsource = GBitmap::create(*ByteStream::create(GURL::Filename::UTF8(argv[1]), "rb"));
    GBitmap &image = *gsource;

    GP<GBitmap> gblack = GBitmap::create();
    GBitmap &black = *gblack;
    GP<GBitmap> gwhite = GBitmap::create();
    GBitmap &white = *gwhite;

    get_image_parts(image, black, white, CELL_SIZE, CELL_SIZE);
    save(black, "black.pgm");
    save(white, "white.pgm");

    GP<GBitmap> gresult = GBitmap::create();
    GBitmap &result = *gresult;
    normalize(image, black, white, result);
    result.save_pgm(*ByteStream::create(GURL::Filename::UTF8(argv[2]), "wb"));

    return 0;
}
