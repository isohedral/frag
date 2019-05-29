SOURCES = $(wildcard *.cpp)
OBJECTS = $(SOURCES:.cpp=.o)
DEPENDS = $(SOURCES:.cpp=.d)
## If you're not on MacOS, you'll need to change this line.
LDFLAGS = -L/opt/local/lib -lglew -lglfw -framework OpengL
## You might need to change /opt/local/include so that you can 
## find GLFW, GLM, etc.
CPPFLAGS = -I/opt/local/include -std=c++11 
CXXFLAGS = $(CPPFLAGS) -W -Wall -O3 -g
CXX = g++

MAIN = frag

all: $(MAIN)

depend: $(DEPENDS)

clean:
	rm -f *.o *.d $(MAIN)

frag: frag.o
	$(CXX) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) -o $@ -c $(CXXFLAGS) $<

%.d: %.cpp
	set -e; $(CC) -M $(CPPFLAGS) $< \
                  | sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@; \
                [ -s $@ ] || rm -f $@

include $(DEPENDS)
