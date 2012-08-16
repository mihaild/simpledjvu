DJVULIBRE_PATH = /home/mihaild/djvulibre
CC = g++ -O3 -std=c++0x -MMD
INCLUDES=-I$(DJVULIBRE_PATH) -I$(DJVULIBRE_PATH)/libdjvu -I$(DJVULIBRE_PATH)/tools
CXXFLAGS=$(INCLUDES) -DHAVE_CONFIG_H -pthread -DTHREADMODEL=POSIXTHREADS
LINK=g++ -O3

BIN_FILES = create_djvu

all: $(BIN_FILES)

create_djvu: build/create_djvu.o build/hystogram_splitter.o build/normalize.o
	$(LINK) -o create_djvu $^ -DHAVE_CONFIG_H -ldjvulibre

build/%.o build/%.d: %.cpp
	$(CC) $(CXXFLAGS) -c -o build/$*.o $*.cpp

clean:
	rm -f build/* $(BIN_FILES)

-include $(wildcard build/*.d)
