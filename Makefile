CXX=i386-mingw32-gcc
CXXFLAGS=-Wall -ggdb  -O2
# -mwindows fixes Undefined reference problems
LDFLAGS=-mwindows
LDLIBS=-lmsimg32 -lstdc++
all: ddb-bench

clean:
	rm ddb-bench
