#Simpledjvu-0.1
#Based on djvulibre (http://djvu.sourceforge.net/)
#Copyright 2012, Mikhail Dektyarev <mihail.dektyarow@gmail.com>

#This file is part of Simpledjvu.

#Simpledjvu is free software: you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation, either version 3 of the License, or
#(at your option) any later version.

#Simpledjvu is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.

#You should have received a copy of the GNU General Public License
#along with Simpledjvu.  If not, see <http://www.gnu.org/licenses/>.


DJVULIBRE_PATH = /home/mihaild/djvulibre
CC = g++ -O3 -std=c++0x -MMD
INCLUDES=-I$(DJVULIBRE_PATH) -I$(DJVULIBRE_PATH)/libdjvu -I$(DJVULIBRE_PATH)/tools -I.
CXXFLAGS=$(INCLUDES) -DHAVE_CONFIG_H -pthread -DTHREADMODEL=POSIXTHREADS
LINK=g++ -O3

BIN_FILES = create_djvu get_pgm_diff

all: $(BIN_FILES)

create_djvu: build/create_djvu.o build/hystogram_splitter.o build/normalize.o build/pgm2jb2.o build/jb2tune.o jb2cmp/libjb2cmp.a
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
