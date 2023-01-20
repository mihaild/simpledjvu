/*
 * Simpledjvu-0.1
 * Based on djvulibre (http://djvu.sourceforge.net/)
 * Copyright 2012, Mikhail Dektyarev <mihail.dektyarow@gmail.com>
 *
 * This file is part of Simpledjvu.
 *
 * Simpledjvu is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Simpledjvu is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Simpledjvu.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <djvulibre.h>

#include <normalize.h>
#include <pgm2jb2.h>

#include <vector>
#include <array>
#include <string>

#include <iostream>
#include <cstdlib>

using std::vector;
using std::array;
using std::string;

const byte THRESHOLD_LEVEL = 128;

struct Keys
{
    bool include_bg;
    bool include_fg;
    int mask_mul;
    int dpi;
    bool use_normalized;
    int normalize_iters;
    int threshold_level;
    int cjb2_loss_level;
    vector<int> slices_bg;
    vector<int> slices_fg;
    Keys():
        include_bg(true),
        include_fg(true),
        mask_mul(2),
        dpi(300),
        use_normalized(false),
        normalize_iters(200),
        threshold_level(128),
        cjb2_loss_level(1),
        slices_bg({74, 89, 99}),
              slices_fg({89})
    {
    }

};

/*
 * it should be const GBitmap, but for unknown reason GBitmap::save_pgm is not declared as const
 */
void save(const GP<GBitmap> gimage, const char *fname, bool pgm = true)
{
    if (pgm)
    {
        gimage->save_pgm(*ByteStream::create(GURL::Filename::UTF8(fname), "wb"));
    }
    else
    {
        gimage->save_pbm(*ByteStream::create(GURL::Filename::UTF8(fname), "wb"));
    }
}

enum Chunk { BACKGROUND, FOREGROUND};

GP<GBitmap> make_chunk_mask(const GBitmap &mask, Chunk chunk)
{
    GP<GBitmap> result = GBitmap::create(mask.rows(), mask.columns());
    result->set_grays(2);
    int ok_color = chunk == BACKGROUND;
    for (int i = 0; i < mask.rows(); ++i)
    {
        for (int j = 0; j < mask.columns(); ++j)
        {
            (*result)[i][j] = mask[i][j] == ok_color ||
                              (i > 0 && mask[i-1][j] == ok_color) ||
                              (i < mask.rows() - 1 && mask[i+1][j] == ok_color) ||
                              (j > 0 && mask[i][j-1] == ok_color) ||
                              (j < mask.columns() - 1 && mask[i][j+1] == ok_color);
        }
    }
    return result;
}

void print_help()
{
    std::cerr
            << "Usage: simpledjvu [options] input.pgm output.djvu\n"
            << "where options =\n"
            << "\t-nobg\n"
            << "\t-nofg\n"
            << "\t-mask_mul n\n"
            << "\t-dpi n\n"
            << "\t-use_normalized\n"
            << "\t-normalize_iters n\n"
            << "\t-threshold_level n\n"
            << "\t-cjb2_loss_level n\n"
            << "\t-slices_bg n1,n2,..\n"
            << "\t-slices_fg n1,n2,...\n"
            ;
}

bool parse_keys(int argc, char *argv[], Keys *keys, char **input, char **output)
{
    if (argc <= 2)
    {
        std::cerr << "Not enough arguments\n";
        return false;
    }
    (*input) = argv[argc - 2];
    (*output) = argv[argc - 1];
    for (int i = 1; i < argc - 2; ++i)
    {
        if (argv[i][0] != '-')
        {
            std::cerr << "Wrong option format: " << argv[i] << '\n';
            return false;
        }
        string arg = argv[i];
        if (arg == "-nobg")
        {
            keys->include_bg = false;
        }
        else if (arg == "-nofg")
        {
            keys->include_fg = false;
        }
        else if (arg == "-use_normalized")
        {
            keys->use_normalized = true;
        }
        else if (arg == "-mask_mul" || arg == "-dpi" || arg == "-normalize_iters" || arg == "-threshold_level" || arg == "-cjb2_loss_level")
        {
            ++i;
            int n;
            char *endptr;
            n = strtol(argv[i], &endptr, 10);
            if (*endptr)
            {
                std::cerr << "Bad number: " << argv[i] << '\n';
                return false;
            }
            if (arg == "-mask_mul")
            {
                keys->mask_mul = n;
            }
            else if (arg == "-dpi")
            {
                keys->dpi = n;
            }
            else if (arg == "-normalize_iters")
            {
                keys->normalize_iters = n;
            }
            else if (arg == "-threshold_level")
            {
                keys->threshold_level = n;
            }
            else if (arg == "-cjb2_loss_level")
            {
                keys->cjb2_loss_level = n;
            }
        }
        else if (arg == "-slices_bg" || arg == "-slices_fg")
        {
            ++i;
            char *nptr = argv[i], *endptr;
            vector<int> ns;
            while (*nptr)
            {
                int n = strtol(nptr, &endptr, 10);
                if (endptr == nptr || *endptr != ',' && *endptr != 0)
                {
                    std::cerr << "Bad numbers: " << argv[i] << '\n';
                    return false;
                }
                ns.push_back(n);
                nptr = *endptr ? endptr + 1 : endptr;
            }
            if (arg == "-slices_bg")
            {
                keys->slices_bg = ns;
            }
            else if (arg == "-slices_fg")
            {
                keys->slices_fg = ns;
            }
        }
        else
        {
            std::cerr << "Unknown option: " << argv[i] << '\n';
            return false;
        }
    }
    return true;
}

/*
 * random parts from c44 tool source code
 * random mix of references and pointers, just like in main djvulibre code
 *
 * @todo: understand, how does it work
 */
void write_part_to_djvu(const GBitmap &image, const vector<int> &slices, const GP<GBitmap> &gmask, IFFByteStream &iff, Chunk chunk)
{
    GP<IW44Image> iw = IW44Image::create_encode(image, gmask);
    vector<IWEncoderParms> parms(slices.size());
    for (int i = 0; i < parms.size(); ++i)
    {
        parms[i].slices = slices[i];
        // is it necessary?
        parms[i].bytes = 0;
        parms[i].decibels = 0;
    }

    for (const auto& parm : parms)
    {
        if (chunk == BACKGROUND)
        {
            iff.put_chunk("BG44");
        }
        else
        {
            iff.put_chunk("FG44");
        }
        iw->encode_chunk(iff.get_bytestream(), parm);
        iff.close_chunk();
    }
}

int main(int argc, char *argv[])
{
    Keys keys;
    char *input, *output;
    if (!parse_keys(argc, argv, &keys, &input, &output))
    {
        print_help();
        return 1;
    }

    GP<GBitmap> gimage = GBitmap::create(*ByteStream::create(GURL::Filename::UTF8(input), "rb"));

    GP<GBitmap> gnormalized_small = get_norm_image(*gimage, keys.normalize_iters);

    GP<GBitmap> gnormalized = GBitmap::create(gimage->rows() * keys.mask_mul, gimage->columns() * keys.mask_mul);
    rescale_bitmap(*gnormalized_small, *gnormalized);

    gnormalized->binarize_grays(keys.threshold_level);

    GP<JB2Image> gmask = pbm2jb2(gnormalized, keys.cjb2_loss_level);

    /*
     * this code is based on djvumake and c44 tools source
     */
    GP<IFFByteStream> giff = IFFByteStream::create(ByteStream::create(GURL::Filename::UTF8(output), "wb"));
    IFFByteStream &iff = *giff;
    iff.put_chunk("FORM:DJVU", 1);

    GP<DjVuInfo> ginfo = DjVuInfo::create();
    ginfo->width = gmask->get_width();
    ginfo->height = gmask->get_height();
    ginfo->dpi = keys.dpi;

    iff.put_chunk("INFO");
    ginfo->encode(*iff.get_bytestream());
    iff.close_chunk();

    iff.put_chunk("Sjbz");
    gmask->encode(iff.get_bytestream());
    iff.close_chunk();

    gnormalized_small->binarize_grays(THRESHOLD_LEVEL);

    GP<GBitmap> gbetter_image;
    if (keys.use_normalized)
    {
        gbetter_image = get_norm_image(*gimage, 2);
    }
    else
    {
        gbetter_image = gimage;
    }

    if (keys.include_fg)
    {
        write_part_to_djvu(*gbetter_image, keys.slices_fg, make_chunk_mask(*gnormalized_small, FOREGROUND), iff, FOREGROUND);
    }
    if (keys.include_bg)
    {
        write_part_to_djvu(*gbetter_image, keys.slices_bg, make_chunk_mask(*gnormalized_small, BACKGROUND), iff, BACKGROUND);
    }

    return 0;
}
