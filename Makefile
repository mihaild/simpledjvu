CC = g++ -O3 -std=c++0x

BIN_FILES = bitonize remove_background decrease_colors_count bitonize_threshold build_hystograms local_threshold classifier_bitonize

all: $(BIN_FILES)

bitonize: build/bitonize.o build/pgm.o
	$(CC) -o bitonize build/bitonize.o build/pgm.o

remove_background: build/remove_background.o build/pgm.o
	$(CC) -o remove_background build/remove_background.o build/pgm.o

decrease_colors_count: build/decrease_colors_count.o build/pgm.o
	$(CC) -o decrease_colors_count build/decrease_colors_count.o build/pgm.o

bitonize_threshold: build/bitonize_threshold.o build/pgm.o
	$(CC) -o bitonize_threshold build/bitonize_threshold.o build/pgm.o

build_hystograms: build/build_hystograms.o build/hystograms.o build/pgm.o
	$(CC) -o build_hystograms build/build_hystograms.o build/hystograms.o build/pgm.o

local_threshold: build/local_threshold.o build/hystograms.o build/pgm.o
	$(CC) -o local_threshold build/local_threshold.o build/hystograms.o build/pgm.o

classifier_bitonize: build/classifier_bitonize.o build/pgm.o build/disjoint_set_forest.o build/connected_components.o build/quality.o
	$(CC) -o classifier_bitonize build/classifier_bitonize.o build/pgm.o build/disjoint_set_forest.o build/connected_components.o build/quality.o

hystogram_splitter: build/hystogram_splitter.o build/pgm.o build/hystograms.o
	$(CC) -o hystogram_splitter build/hystogram_splitter.o build/pgm.o build/hystograms.o

build/bitonize.o: bitonize.cpp types.h constants.h
	$(CC) -c -o build/bitonize.o bitonize.cpp

build/remove_background.o: remove_background.cpp types.h pgm.h constants.h
	$(CC) -c -o build/remove_background.o remove_background.cpp

build/decrease_colors_count.o: decrease_colors_count.cpp types.h pgm.h constants.h
	$(CC) -c -o build/decrease_colors_count.o decrease_colors_count.cpp

build/bitonize_threshold.o: bitonize_threshold.cpp types.h pgm.h constants.h
	$(CC) -c -o build/bitonize_threshold.o bitonize_threshold.cpp

build/build_hystograms.o: build_hystograms.cpp types.h pgm.h constants.h
	$(CC) -c -o build/build_hystograms.o build_hystograms.cpp

build/local_threshold.o: local_threshold.cpp types.h pgm.h constants.h
	$(CC) -c -o build/local_threshold.o local_threshold.cpp

build/classifier_bitonize.o: classifier_bitonize.cpp types.h pgm.h constants.h disjoint_set_forest.h connected_components.h
	$(CC) -c -o build/classifier_bitonize.o classifier_bitonize.cpp

build/hystograms.o: hystograms.cpp types.h pgm.h constants.h
	$(CC) -c -o build/hystograms.o hystograms.cpp

build/pgm.o: pgm.cpp types.h constants.h
	$(CC) -c -o build/pgm.o pgm.cpp

build/disjoint_set_forest.o: disjoint_set_forest.cpp disjoint_set_forest.h
	$(CC) -c -o build/disjoint_set_forest.o disjoint_set_forest.cpp

build/quality.o: quality.cpp quality.h
	$(CC) -c -o build/quality.o quality.cpp

build/connected_components.o: connected_components.cpp connected_components.h quality.h
	$(CC) -c -o build/connected_components.o connected_components.cpp

build/hystogram_splitter.o: hystogram_splitter.cpp pgm.h hystograms.h
	$(CC) -c -o build/hystogram_splitter.o hystogram_splitter.cpp

clean:
	rm -f build/* $(BIN_FILES)
