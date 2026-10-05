#pragma once

#include "bitarray.h"
#include "varray.h"

// Elias-Fano encoding of the non-decreasing values in A
// i.e. a compression routine with O(1) access time
// Works by splitting each number into a lower and upper part
// the lower part is left untouched while the upper part is
// compressed by a unary array in a quite elegant way :)

typedef struct{
    u64 n;
    bitarray * upper;
    varray * lower;
    u64 lower_bits;
    u64 mem_allocated;
} cindex;

cindex * cindex_new(const u32 * A, u32 n);
u32 cindex_get(const cindex * C, u32 kk);
void cindex_free(cindex * C);
