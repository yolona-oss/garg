CC = ccache gcc


LIB_SQLITE  = $(shell pkg-config --libs sqlite3)
LIB_LINCONF = $(shell pkg-config --libs libconfig)

INC     =
LDFLAGS =
LDLIBS  = $(LIB_SQLITE) $(LIB_LINCONF)

CFLAGS  = -g -c -Wall -xc -O0 $(INC)
CFLAGS +=

SOURCES    = main.c util.c eprintf.c gamerec.c scan.c ccread.c
OBJECTS    = $(SOURCES:.c=.o)
EXECUTABLE = garg

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
