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

PROJECT = simpledjvu
DJVULIBRE_PATH = src/djvulibre
CXX = g++ -O3 -std=c++0x
INCLUDES = -I$(DJVULIBRE_PATH) -I$(DJVULIBRE_PATH)/libdjvu -I$(DJVULIBRE_PATH)/tools -Isrc
CXXFLAGS = $(INCLUDES) -DHAVE_CONFIG_H -pthread -DTHREADMODEL=POSIXTHREADS
LDFLAGS = -ldjvulibre
LN = $(CXX) -DHAVE_CONFIG_H
RM = rm -f

OBJ_FILES = src/simpledjvu.o src/hystogram_splitter.o src/normalize.o src/pgm2jb2.o src/jb2tune.o src/jb2cmp/libjb2cmp.a
OBJ_FILE_PGM = src/get_pgm_diff.o
BIN_FILES = $(PROJECT) get_pgm_diff

all: djvulibre_config $(BIN_FILES)

$(PROJECT): $(OBJ_FILES)
	$(LN) $^ $(LDFLAGS) -o $@

get_pgm_diff: $(OBJ_FILE_PGM)
	$(LN) $^ $(LDFLAGS) -o $@

%.o: %.cpp
	$(CC) $(CXXFLAGS) -c $< -o $@

src/jb2cmp/libjb2cmp.a: 
	cd src/jb2cmp && ${MAKE}

djvulibre_config:
	cd src/djvulibre && ./autogen.sh

clean:
	$(RM) $(OBJ_FILES) $(OBJ_FILE_PGM) $(BIN_FILES)
	cd src/jb2cmp && ${MAKE} clean
