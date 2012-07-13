all: bitonize remove_background

bitonize: bitonize.o pgm.o
	gcc -O3 -o bitonize bitonize.o pgm.o

remove_background: remove_background.o pgm.o
	gcc -O3 -o remove_background remove_background.o pgm.o

bitonize.o: bitonize.c types.h
	gcc -O3 -c -o bitonize.o bitonize.c

remove_background.o: remove_background.c types.h
	gcc -O3 -c -o remove_background.o remove_background.c

pgm.o: pgm.c types.h
	gcc -O3 -c -o pgm.o pgm.c

clean:
	rm -f pgm.o bitonize.o remove_background.o bitonize remove_background
