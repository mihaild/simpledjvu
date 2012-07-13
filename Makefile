all: bitonize remove_background decrease_colors_count

bitonize: bitonize.o pgm.o
	gcc -O3 -o bitonize bitonize.o pgm.o

remove_background: remove_background.o pgm.o
	gcc -O3 -o remove_background remove_background.o pgm.o

decrease_colors_count: decrease_colors_count.o pgm.o
	gcc -O3 -o decrease_colors_count decrease_colors_count.o pgm.o

bitonize.o: bitonize.c types.h
	gcc -O3 -c -o bitonize.o bitonize.c

remove_background.o: remove_background.c types.h
	gcc -O3 -c -o remove_background.o remove_background.c

decrease_colors_count.o: decrease_colors_count.c types.h
	gcc -O3 -c -o decrease_colors_count.o decrease_colors_count.c

pgm.o: pgm.c types.h
	gcc -O3 -c -o pgm.o pgm.c

clean:
	rm -f pgm.o bitonize.o remove_background.o decrease_colors_count.o bitonize remove_background decrease_colors_count
