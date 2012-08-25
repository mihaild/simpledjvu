#include <iostream>

#include "djvulibre.h"

#include "pgm2jb2.h"

unsigned int test_size(const GBitmap &image, int threshold) {
    GP<GBitmap> bit_image = GBitmap::create(image);
    bit_image->binarize_grays(threshold);
    GP<JB2Image> jb2_image = pbm2jb2(bit_image, 100);
    return jb2_image->get_memory_usage();
}

unsigned int get_threshold_level(const GBitmap &image) {
    /*for (int i = 0; i < 256; i += 2) {
        std::cerr << i << '\n';
        std::cout << i << ' ' << test_size(image, i) << '\n';
    }*/
    return 128;
}

/*int main(int argc, char *argv[]) {
    GP<GBitmap> gsource = GBitmap::create(*ByteStream::create(GURL::Filename::UTF8(argv[1]), "rb"));
    GBitmap &image = *gsource;
    for (int i = 0; i < 255; i += 1) {
        std::cout << i << ' ' << test_size(image, i) << '\n';
    }
    return 0;
}*/
