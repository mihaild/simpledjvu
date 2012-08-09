#pragma once

#include <stdio.h>
#include "constants.h"
#include "types.h"
#include <string>

const char
    invalid_pgm[] = "Invalid PGM file\n",
    supported_256_only[] = "Only 256-level PGMs are supported\n";

// Loading PGM and freeing pixels {{{

// Skips to the end of line.
void pgm_skip_comment_line(FILE *file);
void pgm_skip_whitespace_and_comments(FILE *file);

// Load a 256-level PGM. Calls exit() on error.
//static void load_pgm(FILE *f);
void load_pgm(FILE *f, int32 *width, int32 *height, int32 *row_size, int32 *rows_count, byte **pixels, int border);
void save_pgm(FILE *f, byte *pixels, int width, int height);
void save_pgm(FILE *f, const GrayImage &image);

void save_pbm(FILE *f, byte *colors, int32 width, int32 height, int32 row_size, int32 rows_count);
void save_pbm(FILE *f, const bitonal_image &image);

GrayImage c_array_to_vector(byte *pixels, int width, int height);
byte *vector_to_c_array(const vector<vector<byte> > &v);
