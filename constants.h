#pragma once

const int COLORS_COUNT = 256;

// Level lines whose level belongs to `contour_levels' (below) are contours.
// CONTOUR_LEVELS_COUNT is the length of `contour_levels'.
// A contour is a cycled path, and may be lightening or darkening.
// Each contour has a `gradient flow', that is,
//    sum (for all contour edges) of
//       difference between the pixel inside and the pixel outside.
//#define CONTOUR_LEVELS_COUNT 8
const int CONTOUR_LEVELS_COUNT = 8;
// QUANT is the same but using STATIC_THRESHOLDS
//#define QUANT 32
const int QUANT = COLORS_COUNT / CONTOUR_LEVELS_COUNT;

//  Darkening contours with level less than this are considered suspicious.
//#define LOW_SUSPICIOUS_THRESHOLD 4
const int LOW_SUSPICIOUS_THRESHOLD = 4;

// Lightening contours with level more than this are considered suspicious.
//#define HIGH_SUSPICIOUS_THRESHOLD 3
const int HIGH_SUSPICIOUS_THRESHOLD = 3;

// A suspicious contour is considered garbage
//    if
//      its gradient flow < FLOW_THRESHOLD
//    or
//      its gradient flow < FLOW_THRESHOLD_PER_EDGE * contour's length

//#define FLOW_THRESHOLD 1000
//#define FLOW_THRESHOLD_PER_EDGE 100
const int FLOW_THRESHOLD = 1000;
const int FLOW_THRESHOLD_PER_EDGE = 100;

// The color of an image's margin (0 is black).
// The value of black favors deletion of black margins on scanned images.
//#define MARGIN_COLOR 0
const int MARGIN_COLOR = 0;
