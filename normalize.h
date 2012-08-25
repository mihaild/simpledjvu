#include "djvulibre.h"

#include "types.h"

/*const int CANONICAL_BLACK_LEVEL = 225;
const int CANONICAL_WHITE_LEVEL = 30;*/

// in usual case, we need threshold between white and black
const int CANONICAL_BLACK_LEVEL = 255;
const int CANONICAL_WHITE_LEVEL = 0;

void normalize(const GBitmap &image, const GBitmap &black, const GBitmap &white, GBitmap &result);
