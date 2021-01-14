CC = gcc
CFLAGS = -Wall
CLIBS = -lcrypto -lssl -lpthread -lev

INCLUDE = $(wildcard ./*.h)
SOURCES = $(wildcard ./*.c)

TARGET = main
OBJECTS = $(patsubst %.c,%.o,$(SOURCES))

$(TARGET) : $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@ $(CLIBS) $(INCLUDE_DIRS) $(LIBDIR)
$(OBJECTS) : %.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@ $(CLIBS) $(INCLUDE_DIRS) $(LIBDIR)

.PHONY : clean
clean:
	rm -rf *.o $(TARGET) 
