DJVULIBRE_PATH = /home/mihaild/djvulibre
CC = g++ -O3 -std=c++0x -MMD
INCLUDES=-I$(DJVULIBRE_PATH) -I$(DJVULIBRE_PATH)/libdjvu -I$(DJVULIBRE_PATH)/tools
CXXFLAGS=$(INCLUDES) -DHAVE_CONFIG_H -pthread -DTHREADMODEL=POSIXTHREADS
LINK=g++ -O3

BIN_FILES = hystogram_splitter normalize

all: $(BIN_FILES)

normalize: build/normalize.o
	$(LINK) -o normalize $^ -DHAVE_CONFIG_H -ldjvulibre

bitonize_threshold: build/bitonize_threshold.o build/pgm.o
	$(LINK) -o bitonize_threshold $^

hystogram_splitter: build/hystogram_splitter.o build/hystograms.o
	$(LINK) -o hystogram_splitter $^ -DHAVE_CONFIG_H -ldjvulibre

build/%.o build/%.d: %.cpp
	$(CC) $(CXXFLAGS) -c -o build/$*.o $*.cpp

clean:
	rm -f build/* $(BIN_FILES)

-include $(wildcard build/*.d)
