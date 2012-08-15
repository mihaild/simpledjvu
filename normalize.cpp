#include <stdlib.h>
#include <stdio.h>
#include <cassert>
#include "types.h"

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

const int CANONICAL_BLACK_LEVEL = 225;
const int CANONICAL_WHITE_LEVEL = 30;

byte canonize_level(const byte raw, const byte black, const byte white) {
    if (raw >= black) {
        if (black == COLORS_COUNT - 1) {
            return CANONICAL_BLACK_LEVEL;
        }
        return CANONICAL_BLACK_LEVEL * raw / (COLORS_COUNT - 1 - black);
    }
    if (raw <= white) {
        if (white == 0) {
            return CANONICAL_WHITE_LEVEL;
        }
        return CANONICAL_WHITE_LEVEL + (COLORS_COUNT - 1 - CANONICAL_WHITE_LEVEL) * (raw - white) / white;
    }
    return CANONICAL_BLACK_LEVEL + (raw - black) * (CANONICAL_WHITE_LEVEL - CANONICAL_BLACK_LEVEL) / (white - black);
}

void normalize(const GBitmap &image, const GBitmap &black, const GBitmap &white, GBitmap &result) {
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

int main(int argc, char **argv) {
    GP<GBitmap> gimage = GBitmap::create(*ByteStream::create(GURL::Filename::UTF8(argv[1]), "rb"));
    GBitmap &image = *gimage;
    GP<GBitmap> gblack = GBitmap::create(*ByteStream::create(GURL::Filename::UTF8(argv[2]), "rb"));
    GBitmap &black = *gblack;
    GP<GBitmap> gwhite = GBitmap::create(*ByteStream::create(GURL::Filename::UTF8(argv[3]), "rb"));
    GBitmap &white = *gwhite;
    GP<GBitmap> gresult = GBitmap::create();
    GBitmap &result = *gresult;
    normalize(image, black, white, result);
    result.save_pgm(*ByteStream::create(GURL::Filename::UTF8(argv[4]), "wb"));

    return 0;
}
