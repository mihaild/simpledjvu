#include "pgm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Building histogram and determining quantization colors /*}}}*/
// Loading PGM and freeing pixels {{{

// Skips to the end of line.
static void pgm_skip_comment_line(FILE *file)/*{{{*/
{
    while (1) switch(fgetc(file))
    {
        case EOF: case '\r': case '\n':
            return;
    }
}/*}}}*/
static void pgm_skip_whitespace_and_comments(FILE *file)/*{{{*/
{
    int c = fgetc(file);
    while(1) switch(c)
    {
        case '#':
            pgm_skip_comment_line(file);
            /* fall through */
        case ' ': case '\t': case '\r': case '\n':
            c = fgetc(file);
        break;
        default:
            ungetc(c, file);
            return;
    }
}/*}}}*/

// Load a 256-level PGM. Calls exit() on error.
void load_pgm(FILE *f, int32 *width, int32 *height, int32 *row_size, int32 *rows_count, byte **pixels, int border)/*{{{*/
{
    int i, in, maxval;

    if (getc(f) != 'P')
    {
        fprintf(stderr, invalid_pgm);
        exit(1);
    }
    if (getc(f) != '5')
    {
        fprintf(stderr, invalid_pgm);
        exit(1);
    }

    pgm_skip_whitespace_and_comments(f);
    fscanf(f, "%d %d %d", width, height, &maxval);
    if (maxval != 255)
    {
        fprintf(stderr, supported_256_only);
        exit(1);
    }

    switch(getc(f))
    {
        case ' ': case '\t': case '\r': case '\n':
            break;
        default:
            fprintf(stderr, invalid_pgm);
            exit(1);
    }

    (*row_size) = (*width) + (border ? 2 : 0);
    (*rows_count) = (*height) + (border ? 2 : 0);
    (*pixels) = (byte *) malloc((*row_size) * (*rows_count));
    if (border) {
        memset((*pixels), MARGIN_COLOR, (*row_size));
        memset((*pixels) + ((*rows_count) - 1) * (*row_size), MARGIN_COLOR, (*row_size));
        (*pixels) += (*width) + 3;
    }

    for (i = 0, in = 0; i < (*height); i++, in += (*row_size))
    {
        fread((*pixels) + in, 1, (*width), f);
        if (border) {
            (*pixels)[in - 1] = (*pixels)[in + (*width)] = MARGIN_COLOR;
        }
    }
}/*}}}*/

void save_pgm(FILE *f, byte *pixels, int width, int height) {
    fprintf(f, "P5\n%d %d\n255\n", width, height);
    fwrite(pixels, 1, width*height, f);
}

static void pack_row(byte *bits, byte *bytes, int n)/*{{{*/
{
    int coef = 0x80;
    int i = n;
    int a = 0;
    while (i--)
    {
        if (*bytes++) a |= coef;

        coef >>= 1;
        if (!coef)
        {
            coef = 0x80;
            *bits++ = a;
            a = 0;
        }
    }
    if (n & 7) *bits = a;
}/*}}}*/

void save_pbm(FILE *f, byte *colors, int32 width, int32 height, int32 row_size, int32 rows_count) {
    int32 packed_row_size = (width + 7) >> 3;
    byte *row = (byte *) malloc(width);
    byte *packed_row = (byte *) malloc(packed_row_size);
    int32 i, in;

    fprintf(f, "P4\n%d %d\n", width, height);

    for (i = 0, in = 0; i < height; i++, in += row_size)
    {
        int j;
        for (j = 0; j < width; j++)
        {
            row[j] = colors[in + j];
        }

        pack_row(packed_row, row, width);
        fwrite(packed_row, 1, packed_row_size, f);
    }

    free(row);
    free(packed_row);
    free(colors);
}
