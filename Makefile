all: bitonize remove_background

bitonize: bitonize.o pgm.o
	gcc --debug -o bitonize bitonize.o pgm.o

remove_background: remove_background.o pgm.o
	gcc --debug -o remove_backgroung remove_backgroung.o pgm.o

bitonize.o: bitonize.c
	gcc --debug -c -o bitonize.o bitonize.c

remove_background.o: remove_backgroung.c
	gcc --debug -c -o remove_backgroung.o remove_backgroung.c

pgm.o: pgm.c
	gcc --debug -c -o pgm.o pgm.c

clean:
	rm -f pgm.o bitonize.o remove_backgroung.o bitonize remove_backgroung
