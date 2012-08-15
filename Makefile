DJVULIBRE_PATH = /home/mihaild/djvulibre
CC = g++ -O3 -std=c++0x -MMD
INCLUDES=-I$(DJVULIBRE_PATH) -I$(DJVULIBRE_PATH)/libdjvu -I$(DJVULIBRE_PATH)/tools
CXXFLAGS=$(INCLUDES) -DHAVE_CONFIG_H -pthread -DTHREADMODEL=POSIXTHREADS
LINK=g++ -O3

BIN_FILES = bitonize normalize decrease_colors_count bitonize_threshold build_hystograms local_threshold classifier_bitonize hystogram_splitter normalize

all: $(BIN_FILES)

bitonize: build/bitonize.o build/pgm.o
	$(LINK) -o bitonize $^

normalize: build/normalize.o build/pgm.o
	$(LINK) -o normalize $^

decrease_colors_count: build/decrease_colors_count.o build/pgm.o
	$(LINK) -o decrease_colors_count $^ 

bitonize_threshold: build/bitonize_threshold.o build/pgm.o
	$(LINK) -o bitonize_threshold $^

build_hystograms: build/build_hystograms.o build/hystograms.o build/pgm.o
	$(LINK) -o build_hystograms $^

local_threshold: build/local_threshold.o build/hystograms.o build/pgm.o
	$(LINK) -o local_threshold $^

classifier_bitonize: build/classifier_bitonize.o build/pgm.o build/disjoint_set_forest.o build/connected_components.o build/quality.o
	$(LINK) -o classifier_bitonize $^

hystogram_splitter: build/hystogram_splitter.o build/hystograms.o
	$(LINK) -o hystogram_splitter $^ -DHAVE_CONFIG_H -ldjvulibre

build/%.o build/%.d: %.cpp
	$(CC) $(CXXFLAGS) -c -o build/$*.o $*.cpp

clean:
	rm -f build/* $(BIN_FILES)

-include $(wildcard build/*.d)
