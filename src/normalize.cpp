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
#include <types.h>
#include <hystogram_splitter.h>

#include <algorithm>

// in usual case, we need threshold between white and black
const byte CANONICAL_BLACK_LEVEL = MAX_COLOR;
const byte CANONICAL_WHITE_LEVEL = MIN_COLOR;

/*
 * hope, that black > white
 * else it can show strange picture, crash, or destroy the Earth
 */
byte canonize_level(const byte raw, const byte black, const byte white)
{
    if (raw >= black)
    {
        return CANONICAL_BLACK_LEVEL;
    }
    if (raw <= white)
    {
        return CANONICAL_WHITE_LEVEL;
    }
    return (raw - white) * (CANONICAL_BLACK_LEVEL - CANONICAL_WHITE_LEVEL) / (black - white);
}

void normalize_parts(const GBitmap &image, const GBitmap &black, const GBitmap &white, GBitmap &result)
{
    int width = image.columns(), height = image.rows();
    result.init(height, width);
    result.set_grays(256);
    int32 i, j;
    for (i = 0; i < height; ++i)
    {
        for (j = 0; j < width; ++j)
        {
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
    GP<GBitmapScaler> gscaler = GBitmapScaler::create(w, h, nw, nh);  // Creates bitmap scaler
    GRect desired(0, 0, nw, nh);   // Desired output = complete bitmap
    GRect provided(0, 0, w, h);    // Provided input = complete bitmap
    gscaler->scale(provided, in, desired, out);  // Rescale
}

GP<GBitmap> get_norm_image(const GBitmap &image, const int iterations)
{
    GP<GBitmap> current = GBitmap::create(image);
    GP<GBitmap> next = GBitmap::create();
    for (int i = 0; i < iterations; ++i)
    {
        GP<GBitmap> gblack_small = GBitmap::create();
        GBitmap &black_small = *gblack_small;
        GP<GBitmap> gwhite_small = GBitmap::create();
        GBitmap &white_small = *gwhite_small;

        get_image_parts(*current, black_small, white_small, CELL_SIZE);

        GP<GBitmap> gblack = GBitmap::create(image.rows(), image.columns());
        GBitmap &black = *gblack;
        GP<GBitmap> gwhite = GBitmap::create(image.rows(), image.columns());
        GBitmap &white = *gwhite;

        rescale_bitmap(black_small, black);
        rescale_bitmap(white_small, white);

        normalize_parts(*current, black, white, *next);

        std::swap(current, next);
    }

    return next;
}
