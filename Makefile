# Uncomment the following line to disable sound, if you don't have the
# SDL_sound library installed for example
#CFLAGS = -DNO_SOUND
#DBG = -g

.SUFFIXES:            # Delete the default suffixes
.SUFFIXES: .c .o .h   # Define our suffix list

.c.o:
	$(CC) -c -o $@ $(CFLAGS) $<

CC = gcc
CFLAGS += -O2 -Wall $(shell sdl-config --cflags --libs) $(DBG)
LDFLAGS = $(shell sdl-config --libs) -lSDL_sound -lm

all: timeless gensprites

timeless: timeless.o data.o sprites.o sound.o song.o scaler.o
	$(CC) $? -o $@ $(LDFLAGS)

timeless.o:	timeless.c timeless.h scaler.h

data.o: data.c timeless.h

sprites.o: sprites.c timeless.h

sound.o: sound.c timeless.h

song.o: song.c timeless.h

scaler.o: scaler.c scaler.h

clean:
	rm -f *.o timeless

test:	all
	./timeless

gensprites: gensprites.c

nsprites.c: gensprites
	gensprites > nsprites.c
	echo Actually you\'d want to rename nsprites.c to sprites.c


WORK = /ram
VER = 1.0.1
DDIR = SDL_timeless-$(VER)

package:  clean
	rm -rf $(WORK)/$(DDIR)
	mkdir $(WORK)/$(DDIR)
	cp *.c *.h Makefile* README $(WORK)/$(DDIR)
	cd $(WORK) && tar czf $(DDIR).tgz $(DDIR)
