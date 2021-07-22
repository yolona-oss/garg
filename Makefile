CC = ccache gcc

INC     = -I/usr/include/gtk-3.0 \
		  -I/usr/include/pango-1.0 \
		  -I/usr/include/glib-2.0 \
		  -I/usr/lib/glib-2.0/include \
		  -I/usr/include/harfbuzz \
		  -I/usr/include/freetype2 \
		  -I/usr/include/libpng16 \
		  -I/usr/include/libmount \
		  -I/usr/include/blkid \
		  -I/usr/include/fribidi \
		  -I/usr/include/cairo \
		  -I/usr/include/lzo \
		  -I/usr/include/pixman-1 \
		  -I/usr/include/gdk-pixbuf-2.0 \
		  -I/usr/include/gio-unix-2.0 \
		  -I/usr/include/cloudproviders \
		  -I/usr/include/atk-1.0 \
		  -I/usr/include/at-spi2-atk/2.0 \
		  -I/usr/include/dbus-1.0 \
		  -I/usr/lib/dbus-1.0/include \
		  -I/usr/include/at-spi-2.0
LDFLAGS =
LDLIBS  = -l sqlite3 \
		  -l gtk-3 \
		  -l gdk-3 \
		  -lz \
		  -lpangocairo-1.0 \
		  -lpango-1.0 \
		  -lharfbuzz \
		  -latk-1.0 \
		  -lcairo-gobject \
		  -lcairo \
		  -lgdk_pixbuf-2.0 \
		  -lgio-2.0 \
		  -lgobject-2.0 \
		  -lglib-2.0

CFLAGS  = -g -c -Wall -xc -O2
		  # -D_DEFAULT_SOURCE -D_XOPEN_SOURCE=600

SOURCES    = src/main.c \
			 src/run/run.c \
			 src/scan/scan.c \
			 src/games/gamerec.c \
			 src/utils/util.c src/utils/eprintf.c \
			 src/utils/list.c \
			 src/utils/time.c \
			 src/db/dbman.c src/db/settings.c src/db/games.c \
			 src/event/event.c src/event/event_handler.c \
			 src/gui/gui.c
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
	$(CC) $(LDFLAGS) $(INC) $(OBJECTS) -o $@ $(LDLIBS)

.c.o:
	$(CC) $(INC) $(CFLAGS) $< -o $@
		
install: all
	cp -f $(EXECUTABLE) /usr/local/bin/$(EXECUTABLE)

deinstall:
	rm -f /usr/local/bin/$(EXECUTABLE)

clean:
	rm -f $(EXECUTABLE) $(OBJECTS)
