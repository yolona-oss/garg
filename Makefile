CC = ccache gcc


LIB_SQLITE  = $(shell pkg-config --libs sqlite3)

INC     =
LDFLAGS =
LDLIBS  = $(LIB_SQLITE)

CFLAGS  = -g -c -Wall -xc -O2 $(INC)
CFLAGS +=

SOURCES    = src/main.c \
			 src/run/run.c \
			 src/scan/scan.c \
			 src/games/gamerec.c \
			 src/utils/util.c src/utils/eprintf.c \
			 src/utils/list.c \
			 src/db/dbman.c src/db/search_cache.c src/db/games_cache.c
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
