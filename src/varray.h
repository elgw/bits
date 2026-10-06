#pragma once
#include "bitarray.h"

typedef struct {
    u64 nbit;
    u64 nel;
    bitarray * B;
    u64 mem_allocated;
} varray;

// v-array consisting of v-bit entries
varray * varray_new(size_t n, u64 nbit);
void varray_free(varray * V);
u64 varray_get(const varray * V, size_t n);
void varray_set(varray * V, size_t n, u64 value);
