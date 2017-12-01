SOURCES=$(wildcard src/*.cpp)
OBJS=$(SOURCES:.cpp=.o)
# the `libtcod-mingw.dll' is in `LIBFLAGS' because MinGW doesn't recognize plain -ltcod-mingw for some reason
LIBFLAGS=-Llib libtcod-mingw.dll -static-libgcc -static-libstdc++

src/%.o : src/%.cpp
	g++ $< -c -o $@ -Iinclude -Wall -g

all : tuto

icon.o : icon.ico icon.rc
	windres icon.rc icon.o

tuto : $(OBJS) icon.o
	g++ $(OBJS) icon.o -o gaunt -Wall -std=c++11 $(LIBFLAGS) -g
