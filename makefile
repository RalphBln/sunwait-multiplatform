C=gcc
CFLAGS=-c -Wall 
LDFLAGS= -lm -lstdc++
SOURCES=sunwait.cpp sunriset.cpp print.cpp sunwait.h sunriset.h print.h
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=sunwait

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm *.o sunwait


