# https://www.gnu.org/software/make/manual/make.html

CFLAGS = -Wall -Wextra -Wconversion
LDFLAGS+=-lm

OPT?=0

ifeq ($(OPT),1)
CFLAGS+=-O3
LDFLAGS+=-flto
else
CFLAGS+=-g3 -Og -fsanitize=address
endif

OBJECTS=bitarray.o \
varray.o \
cindex.o \
rank9.o \
select1.o

smallindex_test: smallindex_test.c $(OBJECTS)
	$(CC) $(CFLAGS) smallindex_test.c $(OBJECTS) $(LDFLAGS) -o smallindex_test

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -rf *.o smallindex_test
