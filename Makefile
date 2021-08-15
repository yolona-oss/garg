CC = ccache gcc

INC     = -I/usr/include/gtk-4.0 -I/usr/include/pango-1.0 -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/include/harfbuzz -I/usr/include/freetype2 -I/usr/include/libpng16 -I/usr/include/libmount -I/usr/include/blkid -I/usr/include/fribidi -I/usr/include/cairo -I/usr/include/lzo -I/usr/include/pixman-1 -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/graphene-1.0 -I/usr/lib/graphene-1.0/include -mfpmath=sse -msse -msse2 -I/usr/include/gio-unix-2.0
LDFLAGS =
LDLIBS  = -lsqlite3 \
		  -lgtk-4 -lpangocairo-1.0 -lpango-1.0 -lharfbuzz -lgdk_pixbuf-2.0 -lcairo-gobject -lcairo -lvulkan -lgraphene-1.0 -lgio-2.0 -lgobject-2.0 -lglib-2.0

CFLAGS  = -g -c -Wall -xc \
		  -D_DEFAULT_SOURCE -D_XOPEN_SOURCE=600
#-O2 

SOURCES    = src/main.c \
			 src/run/run.c src/run/dialog.c src/run/util.c \
			 src/scan/scan.c \
			 src/games/gamerec.c \
			 src/utils/util.c src/utils/eprintf.c \
			 src/utils/gtk_widget_list.c \
			 src/utils/list.c \
			 src/utils/time.c \
			 src/db/dbman.c src/db/settings.c src/db/games.c
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
