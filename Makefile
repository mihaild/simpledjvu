all: bitonize remove_background decrease_colors_count bitonize_threshold build_hystograms local_threshold classifier_bitonize

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

local_threshold: local_threshold.o hystograms.o pgm.o
	g++ -O3 -std=c++0x -o local_threshold local_threshold.o hystograms.o pgm.o

classifier_bitonize: classifier_bitonize.o pgm.o
	g++ -O3 -std=c++0x -o classifier_bitonize classifier_bitonize.o pgm.o

bitonize.o: bitonize.cpp types.h constants.h
	g++ -O3 -std=c++0x -c -o bitonize.o bitonize.cpp

remove_background.o: remove_background.cpp types.h pgm.h constants.h
	g++ -O3 -std=c++0x -c -o remove_background.o remove_background.cpp

decrease_colors_count.o: decrease_colors_count.cpp types.h pgm.h constants.h
	g++ -O3 -std=c++0x -c -o decrease_colors_count.o decrease_colors_count.cpp

bitonize_threshold.o: bitonize_threshold.cpp types.h pgm.h constants.h
	g++ -O3 -std=c++0x -c -o bitonize_threshold.o bitonize_threshold.cpp

build_hystograms.o: build_hystograms.cpp types.h pgm.h constants.h
	g++ -O3 -std=c++0x -c -o build_hystograms.o build_hystograms.cpp

local_threshold.o: local_threshold.cpp types.h pgm.h constants.h
	g++ -O3 -std=c++0x -c -o local_threshold.o local_threshold.cpp

classifier_bitonize.o: classifier_bitonize.cpp types.h pgm.h constants.h
	g++ -O3 -std=c++0x -c -o classifier_bitonize.o classifier_bitonize.cpp

hystograms.o: hystograms.cpp types.h pgm.h constants.h
	g++ -O3 -std=c++0x -c -o hystograms.o hystograms.cpp

pgm.o: pgm.cpp types.h constants.h
	g++ -O3 -std=c++0x -c -o pgm.o pgm.cpp

disjoint_set_forest.o: disjoint_set_forest.cpp disjoint_set_forest.h
	g++ -O3 -std=c++0x -c -o disjoint_set_forest.o disjoint_set_forest.cpp

clean:
	rm -f pgm.o bitonize.o remove_background.o decrease_colors_count.o hystograms.o classifier_bitonize.o disjoint_set_forest.o bitonize remove_background decrease_colors_count bitonize_threshold build_hystograms classifier_bitonize
