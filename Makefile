all: bitonize remove_background decrease_colors_count bitonize_threshold build_hystograms

bitonize: bitonize.o pgm.o
	g++ -O3 -std=c++0x -o bitonize bitonize.o pgm.o

remove_background: remove_background.o pgm.o
	g++ -O3 -std=c++0x -o remove_background remove_background.o pgm.o

decrease_colors_count: decrease_colors_count.o pgm.o
	g++ -O3 -std=c++0x -o decrease_colors_count decrease_colors_count.o pgm.o

bitonize_threshold: bitonize_threshold.o pgm.o
	g++ -O3 -std=c++0x -o bitonize_threshold bitonize_threshold.o pgm.o

build_hystograms: build_hystograms.o hystograms.o pgm.o
	g++ -O3 -std=c++0x -o build_hystograms build_hystograms.o hystograms.o pgm.o

bitonize.o: bitonize.cpp types.h constants.h
	g++ -O3 -std=c++0x -c -o bitonize.o bitonize.cpp

remove_background.o: remove_background.cpp types.h pgm.h constants.h
	g++ -O3 -std=c++0x -c -o remove_background.o remove_background.cpp

decrease_colors_count.o: decrease_colors_count.cpp types.h pgm.h constants.h
	g++ -O3 -std=c++0x -c -o decrease_colors_count.o decrease_colors_count.cpp

bitonize_threshold.o: bitonize_threshold.cpp types.h pgm.h constants.h
	g++ -O3 -std=c++0x -c -o bitonize_threshold.o bitonize_threshold.cpp

build_hystograms.o: build_hystograms.cpp  types.h pgm.h constants.h
	g++ -O3 -std=c++0x -c -o build_hystograms.o build_hystograms.cpp

hystograms.o: hystograms.cpp types.h pgm.h constants.h
	g++ -O3 -std=c++0x -c -o hystograms.o hystograms.cpp

pgm.o: pgm.cpp types.h constants.h
	g++ -O3 -std=c++0x -c -o pgm.o pgm.cpp

clean:
	rm -f pgm.o bitonize.o remove_background.o decrease_colors_count.o hystograms.o bitonize remove_background decrease_colors_count bitonize_threshold build_hystograms
