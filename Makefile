CC = gcc
CFLAGS = -std=c11 -Wpedantic -Wall -Wextra -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wcast-qual -Wnested-externs
LDLIBS = -lm
SRCDIR = src
INCDIR = include
OBJDIR = obj
TARGET = libjson.so
DESTLIB = /usr/local/lib
DESTINC = /usr/local/include/json
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SOURCES))

.PHONY: all release debug test clean

all: release

release: CFLAGS += -O2
release: $(TARGET)

debug: CFLAGS += -DDEBUG -g
debug: $(TARGET)

test: CFLAGS += -DDEBUG -g
test:
	$(CC) -I. $(CFLAGS) *.c -o demo -L../.. -ljson

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDLIBS) -shared -o $(TARGET)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) -I$(INCDIR) $(CFLAGS) -MMD -MP -fPIC -c $< -o $@

$(OBJDIR):
	@mkdir -p $@

-include $(OBJECTS:.o=.d)

install:
	mkdir -p $(DESTLIB) $(DESTINC)
	mv $(TARGET) $(DESTLIB)
	cp $(INCDIR)/*.h $(DESTINC)

uninstall:
	rm -f $(DESTLIB)/$(TARGET)
	rm -rf $(DESTINC)

clean:
	rm -rf $(OBJDIR)
	rm -f $(TARGET)
	find test -name demo -exec rm {} +

