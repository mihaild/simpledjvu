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

#ifndef HYSTOGRAM_SPLITTER_H_
#define HYSTOGRAM_SPLITTER_H_

#include <djvulibre.h>

#include <vector>

using std::vector;

const int CELL_SIZE = 25;

const double QUANTILE = 0.05;

const int MIN_COLORS_DIFF = 50;

void get_image_parts(const GBitmap &image, GBitmap &black_result, GBitmap &white_result, int cell_size = CELL_SIZE);

#endif  // HYSTOGRAM_SPLITTER_H_
