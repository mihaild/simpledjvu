# simpledjvu

Simpledjvu is a simple project for easy converting pgm to djvu.
It uses [djvulibre](http://djvu.sourceforge.net/) for all technichal work and compression.
Really, the only one thing that it does itself is splitting the image to mask, background and foreground.

## Install
I am too stupid to understand, how does autoconf and other such tools work, so you have to change Makefile manually.

In usual case, set DJVULIBRE_PATH to right value is enough.
May be, you will need to change CXXFLAGS - just look with which flags djvulibre compiles on your machine and copy them.

After it, just say `make` in project directory.

If you want, you can change your PATH variable or copy **simpledjvu** binary to any directory already included in your PATH.

You need g++ version supports c++0x standard flag.

## Usage
`simpledjvu [options] **input.pgm** **output.djvu**`

where options =

**-nobg** Do not include background in djvu output.

**-nofg** Do not include foreground in djvu output.

**-mask_mul n** Multiplicate mask size n times.

**-use_normalized** Use normalized image (in which "almost black" and "almost white" colors are exactly black and white) for background and foreground except of original.

**-normalize_iters n** Use *n* normalization iterations for mask (see "Algorithm description").

**-threshold_level n** Use *n* as threshold level for mask (more gives larger mask).

**-cjb2_loss_level n** Use *n* as cjb2 loss level (see djvulibre cjb2 tool description).

**-slices_bg n1,n2,..** Use *n1,n2,...* as number of slices for c44 for background (see djvulibre c44 tool description).

**-slices_fg n1,n2,...** Use *n1,n2,...* as number of slices for c44 for foreground.

You can use imagemagick or any other similar tool to obtain pgm from other format.

## Algorithm description.
Algorithm is very simple, but it gives surprisingly good result for non-pathological images.
As usual, background and foreground are really white and black parts, not paper and letters.

Main part of it is *normalization*.
We take an image, and for every 20x20 square find a 5% and 95% quantiles to get a "black" and "white" images in resolution 20 times less then original.
Increase this images to original image size, and change the image: make absolutely black and white everything that is darker or lighter than this images, and use linear interpolation between them.

Now we repeat this step many times, and result converges to almost black-and-white image, so we can do simple threshold to obtain the mask.

We use c44 for partially masked images using blurred mask for background, and blurred inverted mask for foreground.
