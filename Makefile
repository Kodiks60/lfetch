# Makefile for lfetch

CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = 
PREFIX = /usr/local

TARGET = lfetch
SRC = src/main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET)

install: $(TARGET)
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 755 $(TARGET) $(DESTDIR)$(PREFIX)/bin/

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(TARGET)

# for check
check: $(TARGET)
	./$(TARGET)

.PHONY: all clean install uninstall check
