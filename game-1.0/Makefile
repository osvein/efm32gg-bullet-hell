#
# Simple development makefile
#

#
# add more flags here,
# but don't remove the "+="
#
CFLAGS += -O2 -g -Wall #-Wsign-compare -Wfloat-equal -Wformat-security #-Werror
CPPFLAGS +=
LDFLAGS +=

OBJS = draw.o game.o

all: game


draw.o: draw.c draw.h util.h
game.o: game.c draw.h util.h

game: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS)

clean:
	-rm -f game

install:


.PHONY: all install clean

# end of development makefile
