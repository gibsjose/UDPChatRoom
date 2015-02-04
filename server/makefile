IDIR =
CC = gcc
CFLAGS = -I$(IDIR) -Wall -g -std=c99

ODIR = obj
LDIR = ../lib
LIBS = -lpthread

BIN = server

_DEPS =
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = server.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

all: setup $(BIN)

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(BIN): $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean setup

setup:
	@mkdir -p $(ODIR)

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ $(BIN)
