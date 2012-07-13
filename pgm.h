#pragma once

#include <stdio.h>
#include "types.h"

static const char
    invalid_pgm[] = "Invalid PGM file\n",
    supported_256_only[] = "Only 256-level PGMs are supported\n";

// Loading PGM and freeing pixels {{{

// The color of an image's margin (0 is black).
// The value of black favors deletion of black margins on scanned images.
#define MARGIN_COLOR 0

// Skips to the end of line.
static void pgm_skip_comment_line(FILE *file);
static void pgm_skip_whitespace_and_comments(FILE *file);

// Load a 256-level PGM. Calls exit() on error.
//static void load_pgm(FILE *f);
void load_pgm(FILE *f, int32 *width, int32 *height, int32 *row_size, int32 *rows_count, byte **pixels, int border);
