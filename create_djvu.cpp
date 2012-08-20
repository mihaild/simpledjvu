#include <iostream>

using std::cout;

#include "djvulibre.h"

#include "hystogram_splitter.h"
#include "normalize.h"
#include "select_threshold_level.h"
#include "pgm2jb2.h"

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

    get_image_parts(image, black_small, white_small, CELL_SIZE);
    
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

    GP<GBitmap> gblack_small = GBitmap::create();
    GBitmap &black_small = *gblack_small;
    GP<GBitmap> gwhite_small = GBitmap::create();
    GBitmap &white_small = *gwhite_small;

    get_image_parts(image, black_small, white_small, CELL_SIZE);
    
    GP<GBitmap> gblack = GBitmap::create(image.rows(), image.columns());
    GBitmap &black = *gblack;
    GP<GBitmap> gwhite = GBitmap::create(image.rows(), image.columns());
    GBitmap &white = *gwhite;

    rescale_bitmap(black_small, black);
    rescale_bitmap(white_small, white);

    /*save(black, "black.pgm");
    save(white, "white.pgm");*/

    GP<GBitmap> gnormalized_small = GBitmap::create(image.rows(), image.columns());

    normalize(image, black, white, *gnormalized_small);

    //rescale_bitmap(*get_norm_image(image), *gnormalized);
    GP<GBitmap> gnormalized = GBitmap::create(image.rows() * 2, image.columns() * 2);
    rescale_bitmap(*gnormalized_small, *gnormalized);
    gnormalized->binarize_grays(get_threshold_level(*gnormalized));

    GP<JB2Image> gmask = pbm2jb2(gnormalized, 1);


    /*GP<GBitmap> gblack_real = GBitmap::create(image.rows() / 12, image.columns() / 12);
    GP<GBitmap> gwhite_real = GBitmap::create(image.rows() / 12, image.columns() / 12);*/
    GP<GBitmap> gblack_real = GBitmap::create(image.rows() / 6, image.columns() / 6);
    GP<GBitmap> gwhite_real = GBitmap::create(image.rows() / 6, image.columns() / 6);
    rescale_bitmap(black_small, *gblack_real);
    rescale_bitmap(white_small, *gwhite_real);

    //GP<GBitmap> tmp = GBitmap::create();
    /*get_image_parts(image, *gblack_real, *tmp, 12);
    get_image_parts(image, *tmp, *gwhite_real, 6);*/

    GP<IW44Image> bg = IW44Image::create_encode(*gwhite_real);
    GP<IW44Image> fg = IW44Image::create_encode(*gblack_real);

    /*
     * this code is based on djvumake and c44 tools source
     */
    GP<IFFByteStream> giff = IFFByteStream::create(ByteStream::create(GURL::Filename::UTF8(argv[2]), "wb"));
    IFFByteStream &iff = *giff;
    iff.put_chunk("FORM:DJVU", 1);

    GP<DjVuInfo> ginfo=DjVuInfo::create();
    ginfo->width = gmask->get_width();
    ginfo->height = gmask->get_height();
    ginfo->dpi = 300;

    iff.put_chunk("INFO");
    ginfo->encode(*iff.get_bytestream());
    iff.close_chunk();

    iff.put_chunk("Sjbz");
    gmask->encode(iff.get_bytestream());
    iff.close_chunk();

    //save(*gnormalized, argv[2], false);
    /*GBitmap &result = *gresult;
    result.save_pgm(*ByteStream::create(GURL::Filename::UTF8(argv[2]), "wb"));*/


    return 0;
}
