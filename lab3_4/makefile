CC=gcc
CFLAGS=-fPIC -Wall -Wextra -O2
LDFLAGS=-L. -ldekoder

LIB_NAME=libdekoder.so
TARGET=main

SRC_LIB=dekoder_lib.c
SRC_MAIN=main.c

HDR=dekoder_lib.h

.PHONY: all clean run

all: $(TARGET)

$(LIB_NAME): $(SRC_LIB) $(HDR)
	$(CC) $(CFLAGS) -shared -o $@ $(SRC_LIB)

$(TARGET): $(SRC_MAIN) $(LIB_NAME)
	$(CC) -o $@ $(SRC_MAIN) -I. $(LDFLAGS)

run: all
	LD_LIBRARY_PATH=. ./$(TARGET)

clean:
	rm -f $(TARGET) $(LIB_NAME)

