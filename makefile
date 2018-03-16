SOURCES=$(wildcard src/*.cpp)
OBJS=$(SOURCES:.cpp=.o)
# the `libtcod-mingw.dll' is in `LIBFLAGS' because MinGW doesn't recognize plain -ltcod-mingw for some reason
LIBFLAGS=-Llib libtcod-mingw.dll -static-libgcc -static-libstdc++

src/%.o : src/%.cpp
	g++ $< -c -o $@ -std=c++11 -Iinclude -Wall -g -O3

all : gaunt

clean:
	rm src/*.o

icon.o : icon.ico assets/icon.rc
	windres assets/icon.rc icon.o

gaunt : $(OBJS) icon.o
	g++ $(OBJS) icon.o -o gaunt -Wall -std=c++11 $(LIBFLAGS) -g -O3
