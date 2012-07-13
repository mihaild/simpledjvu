#!/bin/bash

file=$1
thick=$2

file_name=`basename $file`
file_short_name=`echo $file_name|sed 's/\..*//'`
file_name=${file_short_name}_$thick

./bitonize $file $file_name.pbm $thick
c44 -mask ${file_name}.pbm $file ${file_name}.djvu
convert ${file_name}.djvu ${file_name}_bg.pgm
rm ${file_name}.djvu
./remove_background $file ${file_name}_bg.pgm ${file_name}_nobg.pgm
./bitonize ${file_name}_nobg.pgm ${file_name}_nobg.pbm
./decrease_colors_count ${file_name}_nobg.pgm ${file_name}_less_colors.pgm
