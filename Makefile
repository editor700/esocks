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

remove: rm_man
	rm /usr/bin/$(EXECUTABLE)

rm_man:
	rm /usr/share/man/man8/$(EXECUTABLE).8.bz2

binary:
	cp $(EXECUTABLE) /usr/bin

man: man_copy
	bzip2 -f /usr/share/man/man8/$(EXECUTABLE).8

man_copy:
	cp man/esocks.8 /usr/share/man/man8/$(EXECUTABLE).8

clean:
	rm -rf $(EXECUTABLE) src/*.o
