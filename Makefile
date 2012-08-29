DJVULIBRE_PATH = /home/mihaild/djvulibre
CC = g++ -O3 -std=c++0x -MMD
INCLUDES=-I$(DJVULIBRE_PATH) -I$(DJVULIBRE_PATH)/libdjvu -I$(DJVULIBRE_PATH)/tools
CXXFLAGS=$(INCLUDES) -DHAVE_CONFIG_H -pthread -DTHREADMODEL=POSIXTHREADS
LINK=g++ -O3

BIN_FILES = create_djvu

all: $(BIN_FILES)

#select_threshold_level: build/select_threshold_level.o build/pgm2jb2.o build/jb2tune.o jb2cmp/libjb2cmp.a
	#$(LINK) -o select_threshold_level $^ -DHAVE_CONFIG_H -ldjvulibre

create_djvu: build/create_djvu.o build/hystogram_splitter.o build/normalize.o build/select_threshold_level.o build/pgm2jb2.o build/jb2tune.o jb2cmp/libjb2cmp.a
	$(LINK) -o create_djvu $^ -DHAVE_CONFIG_H -ldjvulibre

get_pgm_diff: build/get_pgm_diff.o
	$(LINK) -o get_pgm_diff $^ -DHAVE_CONFIG_H -ldjvulibre

build/%.o build/%.d: %.cpp
	$(CC) $(CXXFLAGS) -c -o build/$*.o $*.cpp

jb2cmp/libjb2cmp.a: 
	cd jb2cmp && ${MAKE}

clean:
	rm -f build/* $(BIN_FILES)
	cd jb2cmp && ${MAKE} clean

-include $(wildcard build/*.d)
