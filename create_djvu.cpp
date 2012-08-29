#include <iostream>
#include <vector>
#include <array>

using std::cout;
using std::vector;

#include "djvulibre.h"

#include "normalize.h"
#include "select_threshold_level.h"
#include "pgm2jb2.h"

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

enum Chunk { BACKGROUND, FOREGROUND};

GP<GBitmap> make_chunk_mask(const GBitmap &mask, Chunk chunk) {
    GP<GBitmap> result = GBitmap::create(mask.rows(), mask.columns());
    result->set_grays(2);
    int ok_color = chunk == BACKGROUND;
    for (int i = 0; i < mask.rows(); ++i) {
        for (int j = 0; j < mask.columns(); ++j) {
            (*result)[i][j] = mask[i][j] == ok_color/* ||
                (i > 0 && mask[i-1][j] == ok_color) ||
                (i < mask.rows() - 1 && mask[i+1][j] == ok_color) ||
                (j > 0 && mask[i][j-1] == ok_color) ||
                (j < mask.columns() - 1 && mask[i][j+1] == ok_color)*/;
        }
    }
    return result;
}

/*
 * random parts from c44 tool source code
 * random mix of references and pointers, just like in main djvulibre code
 *
 * @todo: understand, how does it work
 */
void write_part_to_djvu(const GBitmap &image, const GP<GBitmap> &gmask, IFFByteStream &iff, Chunk chunk) {
    GP<IW44Image> iw = IW44Image::create_encode(image, gmask);
    vector<IWEncoderParms> parms;
    if (chunk == BACKGROUND) {
        const array<int, 3> slices = {74, 89, 99}; // random numbers
        parms.resize(2);
        for (int i = 0; i < 2; ++i) {
            parms[i].slices = slices[i];
            // is it necessary?
            parms[i].bytes = 0;
            parms[i].decibels = 0;
        }
    }
    else {
        parms.resize(1);
        parms[0].slices = 89; // random number
        parms[0].bytes = 0;
        parms[0].decibels = 0;
    }

    for (const auto& parm : parms) {
        if (chunk == BACKGROUND) {
            iff.put_chunk("BG44");
        }
        else {
            iff.put_chunk("FG44");
        }
        iw->encode_chunk(iff.get_bytestream(), parm);
        iff.close_chunk();
    }
}

int main(int argc, char *argv[]) {
    GP<GBitmap> gimage = GBitmap::create(*ByteStream::create(GURL::Filename::UTF8(argv[1]), "rb"));

    GP<GBitmap> gnormalized_small = get_norm_image(*gimage);

    GP<GBitmap> gnormalized = GBitmap::create(gimage->rows() * 2, gimage->columns() * 2);
    rescale_bitmap(*gnormalized_small, *gnormalized);

    save(*gnormalized, "norm.pgm");

    int threshold_level = get_threshold_level(*gnormalized);

    gnormalized->binarize_grays(threshold_level);

    GP<JB2Image> gmask = pbm2jb2(gnormalized, 1);

    /*
     * this code is based on djvumake and c44 tools source
     */
    GP<IFFByteStream> giff = IFFByteStream::create(ByteStream::create(GURL::Filename::UTF8(argv[2]), "wb"));
    IFFByteStream &iff = *giff;
    iff.put_chunk("FORM:DJVU", 1);

    GP<DjVuInfo> ginfo = DjVuInfo::create();
    ginfo->width = gmask->get_width();
    ginfo->height = gmask->get_height();
    ginfo->dpi = 300;

    iff.put_chunk("INFO");
    ginfo->encode(*iff.get_bytestream());
    iff.close_chunk();

    iff.put_chunk("Sjbz");
    gmask->encode(iff.get_bytestream());
    iff.close_chunk();

    gnormalized_small->binarize_grays(threshold_level);

    write_part_to_djvu(*gimage, make_chunk_mask(*gnormalized_small, FOREGROUND), iff, FOREGROUND);
    write_part_to_djvu(*gimage, make_chunk_mask(*gnormalized_small, BACKGROUND), iff, BACKGROUND);

    return 0;
}
