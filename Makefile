CC=gcc
CFLAGS=-c -O2 -DSOCKS4 -DSOCKS5
LDFLAGS=-lpthread
SOURCES=src/esocks.c src/socks.c src/net.c src/utils.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=esocks

all: $(SOURCES) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

install: binary man

binary:
	cp esocks /usr/bin

man: man_move
	bzip2 /usr/share/man/man8/esocks.8

man_move:
	mv man/esocks.8 /usr/share/man/man8/

clean:
	rm -rf esocks src/*.o
