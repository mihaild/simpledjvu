#if 0   /* Self-compilation, use "chmod +x bitonize.c" to enable */

    C="gcc -Wall -O2 -DNDEBUG bitonize.c -o bitonize"
    echo $C
    $C || exit
    C="strip bitonize"
    echo $C
    $C
    exit

#endif

#define STATIC_THRESHOLDS
    
// this file is best viewed with Vim because of fold markers ({{{ and }}})

// Introduction {{{

// This is `bitonize', the program to get bitonal files out of grayscale ones.
// We assume that the file was bitonal but then spoiled (e.g. by scanner).
// Here the job is not to dither grayshades, but rather to capture sharp edges.

// Author: Ilya Mezhirov
// Special thanks to: Alexander Shen
    
// The job is done in four parts:
//
//  1) load
//  2) get the tree of contours
//  3) paint the tree of contours in black and white
//  4) save

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "pgm.h"
#include "types.h"

// All the program messages. Not a sophisticated interface, eh?.
static const char
    usage[] = "usage: bitonize [<PGM input> [<PBM output>]]\n";

// Level lines whose level belongs to `contour_levels' (below) are contours.
// CONTOUR_LEVELS_COUNT is the length of `contour_levels'.
// A contour is a cycled path, and may be lightening or darkening.
// Each contour has a `gradient flow', that is,
//    sum (for all contour edges) of
//       difference between the pixel inside and the pixel outside.
#define CONTOUR_LEVELS_COUNT 8
// QUANT is the same but using STATIC_THRESHOLDS
#define QUANT 32

//  Darkening contours with level less than this are considered suspicious.
#define LOW_SUSPICIOUS_THRESHOLD 4

// Lightening contours with level more than this are considered suspicious.
#define HIGH_SUSPICIOUS_THRESHOLD 3

// A suspicious contour is considered garbage
//    if
//      its gradient flow < FLOW_THRESHOLD
//    or
//      its gradient flow < FLOW_THRESHOLD_PER_EDGE * contour's length

#define FLOW_THRESHOLD 1000
#define FLOW_THRESHOLD_PER_EDGE 100

static int32 width, height; // of the image, excluding margin

// Dimensions of the `pixels' array.
//    row_size is always width + 2, which means 1 pixel margin on each side.
//    row_count is always height + 2, which means the same.

static int32 row_size, rows_count;

// This array is shifted so that pixels[0] is (0,0) image point.
static byte *pixels; // rows_count rows of row_size pixels each

typedef struct
{
    int32 parent; // the innermost contour enclosing this one
    int32 gradient_flow; // (see above); positive for whitening contours
    int32 length; // should be even
    byte color; // 0/1; filled after all the contours have been discovered
    byte level; // index to `contour_levels' array
} Contour;

// Introduction }}}
// Building histogram and determining quantization colors/*{{{*/

#ifndef STATIC_THRESHOLDS

int32 *histogram;
byte contour_levels[CONTOUR_LEVELS_COUNT];

// Allocate and fill in the histogram.
static void calculate_histogram()
{
    int32 x, y;
    byte *p = pixels;
    histogram = (int32 *) calloc(256, sizeof(int32));
    for (y = height; y; y--)
    {
        for (x = width; x; x--) histogram[*p++]++;
        p += 2; // skip two margin pixels
    }
}

static void find_contour_levels()
{
    int32 total_pixels_count = width * height;
    int32 pixels_per_level = total_pixels_count / CONTOUR_LEVELS_COUNT;
    byte current_color = 0;
    int32 pixels_adopted = 0;
    int i;

    calculate_histogram();
    for (i = 0; i < CONTOUR_LEVELS_COUNT; i++)
    {
        int32 pixels_to_adopt = (i + 1) * pixels_per_level;
        while (current_color < 255 && pixels_adopted < pixels_to_adopt)
        {
            pixels_adopted += histogram[current_color++];
        }
        contour_levels[i] = current_color;
        // contour_levels[i] = i * 255 / (CONTOUR_LEVELS_COUNT - 1);
    }
    free(histogram);
}

#endif // ifndef STATIC_THRESHOLDS

// Building histogram and determining quantization colors /*}}}*/

static void clean_up_pixels()
{
    free(pixels - width - 3);
}

// Loading PGM and freeing pixels }}}

/* This array contains height rows of row_size integers each.
 * For every pixel whose left border belongs to a contour already known,
 *     the value of `pixel_contours' should be the index of
 *         the innermost known contour containing this pixel.
 * For all other pixels, it should be 0.
 */
static int32 *pixel_contours;

static Contour *contours;
static int32 contours_allocated, contours_count;

static int32 allocate_contour()/*{{{*/
{
    if (contours_count == contours_allocated)
    {
        contours_allocated <<= 1;
        contours = (Contour *) realloc
            (contours, contours_allocated * sizeof(Contour));
    }
    return contours_count++;
}/*}}}*/
static void fill_in_left_column()/*{{{*/
{
    int i, in = 0;
    for (i = 0; i < height; i++, i+=row_size)
        pixel_contours[in] = 1; // indicates that all pixels belong to the root
}/*}}}*/
static void initialize()/*{{{*/
{
    pixel_contours = (int32 *) calloc(row_size * height, sizeof(int32)) + 1;
    fill_in_left_column();
    contours_allocated = 256; contours_count = 0;
    contours = (Contour *) malloc(contours_allocated * sizeof(Contour));
    allocate_contour(); // 0th contour is dummy
    allocate_contour(); // 1st contour is the root
    contours[1].level = 0;
}/*}}}*/

// Finding contours {{{

// Reallocs the contours' memory chunk to minimum really used.
static void freeze_contours()/*{{{*/
{

    contours = (Contour *) realloc
        (contours, contours_count * sizeof(Contour));
}/*}}}*/

// Constructs a path starting from the right border of (x_start,y_start) pixel,
// of level `level' and assuming the innermost containing contour is `parent'.
static int32 build_path(int32 x_start, int32 y_start, byte level, int32 parent)/*{{{*/
{
    int32 x1, y1, x2, y2, dx, dy, gradient_flow = 0;
    int32 length = 0;
    int32 x2_start, y2_start, dy_start;
    int32 contour = allocate_contour();
    int clockwise;

    y_start *= row_size;
    clockwise = pixels[y_start + x_start] < pixels[y_start + x_start - 1];

    dx = 0;
    y1 = y2 = y_start;

    if (clockwise)
    {
        x1 = x_start - 1;  // |   ^   |
        x2 = x_start;      // | 1 | 2 |
        dy = -row_size;    // |   |   |
        x_start = x1;
    }
    else
    {
        x1 = x_start;      // |   |   |
        x2 = x_start - 1;  // | 2 | 1 |
        dy = row_size;     // |   V   |
    }
    // now x_start == x1 && y_start == y1
    x2_start = x2; y2_start = y2;
    dy_start = dy;

    // This loop is dangerous - if something is wrong, it hangs.
    do // until return to the starting point
    {
        int c3, c4;
        length++;
        gradient_flow += pixels[x1 + y1] - pixels[x2 + y2];
        if (dy == dy_start)
        {
            if (clockwise)
            {
                pixel_contours[x2 + y2] = contour;
            }
            else
            {
                pixel_contours[x1 + y1] = contour;
            }
        }
        else if (dy == -dy_start)
        {
            if (clockwise)
            {
                if (!pixel_contours[x1 + y1])
                    pixel_contours[x1 + y1] = parent;
            }
            else
            {
                if (!pixel_contours[x2 + y2])
                    pixel_contours[x2 + y2] = parent;
            }
        }

        // Continue the path {{{

        /* (x1,y1) is a (more white) pixel on the left side of the current vector
         * (x2,y2) is a (more black) pixel on the right side of the current vector
         *
         * On this picture dx = 1, dy = 0:
         *
         *  "first"  (x1, y1) ->    | W | ? |   <- (x1 + dx, y1 + dy) "third"
         *                          ===>+----
         *  "second" (x2, y2) ->    | B | ? |   <- (x2 + dx, y2 + dy) "fourth"
         *
         * y1, y2 and dy are scaled by row_size.
         */

        c3 = pixels[y1 + dy + x1 + dx] >= level; // true if 3rd pixel is white
        c4 = pixels[y2 + dy + x2 + dx] >= level; // true if 4th pixel is white
        if (c3 && !c4)
        {
            // go straight
            // the third becomes the first, the fourth becomes the second
            x1 += dx; y1 += dy;
            x2 += dx; y2 += dy;            
        }
        else if (c4 && (c3 || clockwise))
        {
            // turn right
            // the fourth pixel becomes the first
            int new_dx = x2 - x1;
            int new_dy = y2 - y1;
            x1 = x2 + dx; y1 = y2 + dy;
            dx = new_dx; dy = new_dy;
        }
        else
        {
            // turn left
            // the third pixel becomes the second
            int new_dx = x1 - x2;
            int new_dy = y1 - y2;
            x2 = x1 + dx; y2 = y1 + dy;
            dx = new_dx; dy = new_dy;            
        }

        // Continue the path }}}
    }
    while (!(x1 == x_start && y1 == y_start
             && x2 == x2_start && y2 == y2_start)); // until cycled

    if (clockwise) gradient_flow = -gradient_flow;

    contours[contour].gradient_flow = gradient_flow;
    contours[contour].length = length;
    contours[contour].parent = parent;
    return contour;
}/*}}}*/

#ifndef STATIC_THRESHOLDS
int get_level_by_color(byte color, int level)
{
    // It was just "color / CONTOURS_QUANT" before dynamic quantization
    if (color < contour_levels[level])
    {
        while (level > 0 && color < contour_levels[level])
        {
            level--;
        }
    }
    while (level < CONTOUR_LEVELS_COUNT - 1 && color >= contour_levels[level + 1])
    {
        level++;
    }
    return level;
}
#endif

static void find_contours()/*{{{*/
{
    /* Move cursor through the image, discovering contours */
    int32 x = 0, y = 0, c;
    byte *p = pixels;
    int32 *pc = pixel_contours;
    int32 contour;

    for (y = 0; y < height; y++)
    {
        // We start each row from the root contour, with black current color.
        contour = 1; c = 0;
        for (x = 0; x < width; x++)
        {
            #ifdef STATIC_THRESHOLDS            
                byte next_pixel_level = *p / QUANT; 
            #else
                byte next_pixel_level = get_level_by_color(*p, c);
            #endif
            if (next_pixel_level != c)
            {
                int known_level;
                if (*pc)
                {
                    contour = *pc;
                    known_level = contours[*pc].level;
                }
                else
                    known_level = c;

                while (next_pixel_level != known_level)
                {
                    int level, boundary;

                    if (known_level < next_pixel_level)
                    {
                        level = known_level + 1;
                        #ifdef STATIC_THRESHOLDS
                            boundary = level * QUANT;
                        #else
                            boundary = contour_levels[level];
                        #endif
                    }
                    else
                    {
                        level = known_level - 1;
                        #ifdef STATIC_THRESHOLDS
                            boundary = known_level * QUANT;
                        #else
                            boundary = contour_levels[known_level];
                        #endif
                    }

                    contour = build_path(x, y, boundary, contour);
                    contours[contour].level = level;
                    known_level = level;
                }
                c = next_pixel_level;
            }
            p++;
            pc++;
        } // for x in 0..width - 1
        p += 2;
        pc += 2;
    } // for y in 0..height-1
    freeze_contours();
}/*}}}*/

// Finding contours }}}

static void bitonize_contours()/*{{{*/
{
    double *bonus_if_0 = (double *) calloc(contours_count, sizeof(double));
    double *bonus_if_1 = (double *)calloc(contours_count, sizeof(double));
    byte *preference = (byte *) malloc(contours_count);
    int32 i;

    // Go from the leaves to the root. Fill bonus_if_0 and bonus_if_1.
    // Also fill `preference'. (Do we really need it?)
    // Each node contributes to the parent's bonus_if_0 and bonus_if_1.
    // (bonus_if_X means max bonus in the subtree in case this node is X)
    
    for (i = contours_count - 1; i > 1; i--)
    {
        double my_bonus_if_0 = bonus_if_0[i];
        double my_bonus_if_1 = bonus_if_1[i];
        double my_pure_bonus = contours[i].gradient_flow;
        double my_max_bonus;

        if (my_pure_bonus < 0)
        {
            // blackening contour
            int fignya = 0; // if it's garbage
            my_pure_bonus = -my_pure_bonus;
            
            // Decide if it's garbage
            if (contours[i].level > HIGH_SUSPICIOUS_THRESHOLD)
                if (my_pure_bonus < FLOW_THRESHOLD
                        || my_pure_bonus <
                           FLOW_THRESHOLD_PER_EDGE * contours[i].length
                    )
                    fignya = 1;
            
            my_max_bonus = my_pure_bonus + my_bonus_if_1;
            if (fignya || my_max_bonus <= my_bonus_if_0)
            {
                preference[i] = 0;
                my_max_bonus = my_bonus_if_0;
            }
            else
            {
                preference[i] = 1;
            }
            bonus_if_0[contours[i].parent] += my_max_bonus;
            bonus_if_1[contours[i].parent] += my_bonus_if_1;
        }
        else
        {
            // lightening contour
            int fignya = 0;
            
            // Decide if it's garbage
            if (contours[i].level < LOW_SUSPICIOUS_THRESHOLD)
                if (my_pure_bonus < FLOW_THRESHOLD
                        || my_pure_bonus <
                           FLOW_THRESHOLD_PER_EDGE * contours[i].length
                    )
                    fignya = 1;
            
            my_max_bonus = my_pure_bonus + my_bonus_if_0;
            if (fignya || my_max_bonus < my_bonus_if_1)
            {
                preference[i] = 1;
                my_max_bonus = my_bonus_if_1;
            }
            else
            {
                preference[i] = 0;
            }
            bonus_if_0[contours[i].parent] += my_bonus_if_0;
            bonus_if_1[contours[i].parent] += my_max_bonus;
        }
    } // for (all contours)

    // Go from the root to the leaves. Fill `color' based on `preference'.
    
    contours[1].color = 0;
    for (i = 2; i < contours_count; i++)
    {
        byte parent_color = contours[contours[i].parent].color;

        if (contours[i].gradient_flow < 0)
        {
            // blackening contour
            if (parent_color)
                contours[i].color = 1;
            else
                contours[i].color = preference[i];
        }
        else
        {
            // whitening contour
            if (!parent_color)
                contours[i].color = 0;
            else
                contours[i].color = preference[i];
        }
    }

    free(bonus_if_0);
    free(bonus_if_1);
    free(preference);
}/*}}}*/

byte* get_colors(void) {
    byte *colors;
    colors = (byte *) malloc(row_size * height * sizeof(byte));
    int32 i, in,j, contour_index;
    byte color = 0;
    for (i = 0, in = 0; i < height; ++i, in += row_size) {
        for (j = 0; j < width; ++j) {
            if (contour_index = pixel_contours[in + j]) {//=, not ==
                color = contours[contour_index].color;
            }
            colors[in + j] = color;
        }
    }
    return colors;
}

void blow_mask(byte *colors, int thickness) {
    int i, in, j;
    int k, l;
    byte color;
    byte *tmp_colors;
    tmp_colors = (byte *) malloc(row_size * height * sizeof(byte));
    memcpy(tmp_colors, colors, row_size * height * sizeof(byte));
    for (i = 0, in = 0; i < height; ++i, in += row_size) {
        for (j = 0; j < width; ++j) {
            color = 0;
            for (k = -thickness; k <= thickness; ++k) {
                if (i + k >= 0 && i + k < height) {
                    for (l = -thickness; l <= thickness; ++l) {
                        /*printf("%d %d %d\n", k, l, k*k+l*l < thickness*thickness);*/
                        if (k*k + l*l <= thickness*thickness) {
                            if (l + j >= 0 && l + j < width) {
                                color = color || colors[(i + k)*row_size + (j + l)];
                            }
                        }
                    }
                }
            }
            tmp_colors[in + j] = color;
        }
    }
    memcpy(colors, tmp_colors, row_size * height * sizeof(byte));
    free(tmp_colors);
}

// Writing PBM file {{{

static void produce_pbm(FILE *f, int blow_thick)/*{{{*/
{
    byte *colors = get_colors();
    blow_mask(colors, blow_thick);
    save_pbm(f, colors, width, height, row_size, rows_count);
}/*}}}*/

// Writing PBM file }}}
static void clean_up()/*{{{*/
{
    free(contours);
    free(pixel_contours - 1);
    // `pixels' is freed before in clean_up_pixels()
}/*}}}*/
int main(int argc, char **argv)/*{{{*/
{
    FILE *input, *output;
    assert(sizeof(byte) == 1);
    assert(sizeof(int32) == 4);

    if (argc > 4)
    {
        fputs(usage, stderr);
        return 2;
    }

    if (argc >= 2 && strcmp(argv[1], "-"))
    {
        input = fopen(argv[1], "rb");
        if (!input)
        {
            perror(argv[1]);
            exit(1);
        }
    }
    else
    {
        input = stdin;
    }

    if (argc >= 3 && strcmp(argv[2], "-"))
    {
        output = fopen(argv[2], "wb");
        if (!output)
        {
            perror(argv[2]);
            exit(1);
        }
    }
    else
    {
        output = stdout;
    }

    int blow_thick;

    if (argc >= 4) {
        blow_thick = atoi(argv[3]);
    }
    else {
        blow_thick = 0;
    }

    load_pgm(input, &width, &height, &row_size, &rows_count, &pixels, 1);
    initialize();

    if (input != stdin) fclose(input);

    #ifndef STATIC_THRESHOLDS
        find_contour_levels();
    #endif
    find_contours();
    clean_up_pixels();

    bitonize_contours();

    produce_pbm(output, blow_thick);
    if (output != stdout) fclose(output);

    clean_up();

    return 0;
}/*}}}*/
