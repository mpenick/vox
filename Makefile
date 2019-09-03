CXXFLAGS=-O0 -g -I/usr/include/SDL2
LDFLAGS=-lSDL2

all: vox

vox: main.o
	$(CXX) -o vox $< $(LDFLAGS)

clean:
	rm -rf vox *.o
