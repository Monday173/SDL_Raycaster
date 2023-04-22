CC = gcc
CFLAGS = -Wall
LDFLAGS = -lSDL2 -lm
SRC = $(wildcard src/*.c) 
OBJ = $(SRC: .c=.o)
EXEC = bin/exec

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJ) $(LDFLAGS)

clean:
	rm -f $(EXEC) *~
