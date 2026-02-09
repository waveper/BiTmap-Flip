CC = gcc
CFLAGS = -Wall -Wextra
TARGET = btf
SRC = main.c

all: build

build:
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

.PHONY: clean

clean:
	rm btf
