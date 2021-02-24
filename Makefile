CC = ccache gcc

INC     =
LDFLAGS =
LDLIBS  = -l sqlite3 \
		  -l ncursesw \
		  -l menu

CFLAGS  = -g -c -Wall -xc -O2 \
		  -D_DEFAULT_SOURCE -D_XOPEN_SOURCE=600

SOURCES    = src/main.c \
			 src/run/run.c \
			 src/scan/scan.c \
			 src/games/gamerec.c \
			 src/utils/util.c src/utils/eprintf.c \
			 src/utils/list.c \
			 src/utils/time.c \
			 src/db/dbman.c src/db/settings.c src/db/games.c \
			 src/event/event.c src/event/event_handler.c \
			 src/tui/tui.c src/tui/util.c src/tui/input.c
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
