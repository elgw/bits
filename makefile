# https://www.gnu.org/software/make/manual/make.html

CFLAGS = -Wall -Wextra -Wconversion
LDFLAGS+=-lm

OPT?=0
SAN?=0

ifeq ($(OPT),1)
CFLAGS+=-O3 -DNDEBUG
LDFLAGS+=-flto
else
CFLAGS+=-g3 -Og
ifeq ($(SAN),1)
CFLAGS+=-fsanitize=address
endif
endif

OBJECTS=bitarray.o \
varray.o \
cindex.o \
rank1.o \
select1.o

FILES=bitarray_ut.c \
varray_ut.c \
cindex_ut.c \
rank1_ut.c \
select1_ut.c


smallindex_test: smallindex_test.c $(OBJECTS) $(FILES)
	$(CC) $(CFLAGS) smallindex_test.c $(FILES) $(OBJECTS) $(LDFLAGS) -o smallindex_test

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -rf *.o smallindex_test
