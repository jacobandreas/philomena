CC = gcc
CFLAGS = -Wall
LDFLAGS =
SOURCES = philomena.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = philomena

all: $(SOURCES) $(EXECUTABLE)

$(SOURCES):
	$(CC) $(CFLAGS) $@.c

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

%.obj : %.c
	$(CC) $(CFLAGS) -c $(.SOURCE)

clean:
	rm philomena *.o

install:

	cp $(EXECUTABLE) /usr/local/bin
