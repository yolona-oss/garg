CC=ccache gcc

INC     =
LDFLAGS =
LDLIBS  = -l config #$(shell `pkg-config --libs libconfig`)

CFLAGS  = -g -c -Wall -xc -O2 $(INC)
CFLAGS +=

SOURCES =main.c util.c ccread.c
OBJECTS =$(SOURCES:.c=.o)
EXECUTABLE =$(shell basename `pwd`)

all: options $(SOURCES) $(EXECUTABLE)

remake: clean all

options:
	@echo build options:
	@echo "CFLAGS	= $(CFLAGS)"
	@echo "LDFLAGS	= $(LDFLAGS)"
	@echo "CC	    = $(CC)"
		
$(EXECUTABLE): $(OBJECTS) 
		$(CC) $(LDFLAGS) $(OBJECTS) -o $@ $(LDLIBS)

.c.o:
		$(CC) $(CFLAGS) $< -o $@
		
install: all
	cp -f $(EXECUTABLE) /usr/local/bin/$(EXECUTABLE)

deinstall:
	rm -f /usr/local/bin/$(EXECUTABLE)

clean:
	rm -f $(EXECUTABLE) $(OBJECTS)
