SOURCES := $(wildcard *.cpp)
OBJECTS := $(patsubst %.cpp,%.o,$(SOURCES))
CXXFLAGS := -O3 -g -I/usr/include/SDL2
LDFLAGS := -lSDL2 -lepoxy

all: vox

vox: $(OBJECTS)
	$(CXX) -o vox $(OBJECTS) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -rf vox *.o
