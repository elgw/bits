# https://www.gnu.org/software/make/manual/make.html

CFLAGS = -Wall -Wextra -Wconversion -march=native
LDFLAGS+=-lm -lpthread

CFLAGS += -Isrc/

OPT?=0
SAN?=0

ifeq ($(OPT),1)
CFLAGS += -O3 -DNDEBUG
LDFLAGS+=-flto
else
CFLAGS += -g3 -Og
ifeq ($(SAN),1)
CFLAGS += -fsanitize=address
endif
endif

OBJECTS=bitarray.o \
varray.o \
cindex.o \
rank1.o \
bitarray_ut.o \
varray_ut.o \
cindex_ut.o \
rank1_ut.o \
select1.o \
select1_ut.o \
select1c.o \
select1c_ut.o \
select1d.o \
select1d_ut.o

SRCDIR=src/

bits: test/bits.c $(OBJECTS)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

%.o: $(SRCDIR)%.c
	$(CC) -c $(CFLAGS) $<

clean:
	rm -rf *.o bits
