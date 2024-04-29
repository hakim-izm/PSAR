CC ?= gcc
PKGCONFIG = $(shell which pkg-config)
CFLAGS = $(shell $(PKGCONFIG) --cflags gtk4)
LIBS = $(shell $(PKGCONFIG) --libs gtk4)
GLIB_COMPILE_RESOURCES = $(shell $(PKGCONFIG) --variable=glib_compile_resources gio-2.0)
GLIB_COMPILE_SCHEMAS = $(shell $(PKGCONFIG) --variable=glib_compile_schemas gio-2.0)

API_PATH = api
SRC = fileeditorconnect.c fileeditorprefs.c fileeditorwin.c fileeditor.c main.c $(API_PATH)/client.c
BUILT_SRC = resources.c

OBJS = $(BUILT_SRC:.c=.o) $(SRC:.c=.o)

all: fileeditor

com.psar.fileeditor.gschema.valid: com.psar.fileeditor.gschema.xml
	$(GLIB_COMPILE_SCHEMAS) --strict --dry-run --schema-file=$< && mkdir -p $(@D) && touch $@

gschemas.compiled: com.psar.fileeditor.gschema.valid
	$(GLIB_COMPILE_SCHEMAS) .

resources.c: fileeditor.gresource.xml window.ui menu.ui prefs.ui connect.ui
	$(GLIB_COMPILE_RESOURCES) fileeditor.gresource.xml --target=$@ --sourcedir=. --generate-source

$(API_PATH)/%.o: $(API_PATH)/%.c
	$(CC) -c -o $@ $(CFLAGS) $<

%.o: %.c
	$(CC) -c -o $(@F) $(CFLAGS) $<

fileeditor: $(OBJS) gschemas.compiled
	$(CC) -o $(@F) $(OBJS) $(LIBS)

partclean:
	rm -f com.psar.fileeditor.gschema.valid
	rm -f gschemas.compiled
	rm -f $(BUILT_SRC)
	rm -f $(OBJS)

clean:
	rm -f com.psar.fileeditor.gschema.valid
	rm -f gschemas.compiled
	rm -f $(BUILT_SRC)
	rm -f $(OBJS)
	rm fileeditor