CC = gcc
CFLAGS = -Wall

TARGET = pgc

CLEAN_FILES = $(TARGET) *.dat

.PHONY: all clean

all: $(TARGET)

$(TARGET): main.c
	$(CC) -Wall $< -o $@

clean:
	rm -f $(CLEAN_FILES)