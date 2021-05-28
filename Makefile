CXXFLAGS = -O2 -Wall -Iinclude -std=c++11
LDLIBS = -lggwave

all: ggwave-fm

ggwave-fm: ggwave-fm.cpp dsp.cpp

clean:
	rm -f ggwave-fm *.o
