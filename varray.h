#pragma once
#include "bitarray.h"

typedef struct {
    u32 nbit;
    u32 nel;
    bitarray * B;
    u32 mem_allocated;
} varray;

// v-array consisting of v-bit entries
varray * varray_new(size_t n, u32 nbit);
void varray_free(varray * V);
u32 varray_get(const varray * V, size_t n);
void varray_set(varray * V, size_t n, u32 value);
