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

#ifndef PGM2JB2_H_
#define PGM2JB2_H_

#include <djvulibre.h>

GP<JB2Image> pbm2jb2(const GP<GBitmap> &image, int losslevel = 0, int dpi = 300);

#endif  // PGM2JB2_H_
