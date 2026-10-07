#pragma once

#include "bitarray.h"
#include "varray.h"
#include "select1c.h"
// Elias-Fano encoding of the non-decreasing values in A
// i.e. a compression routine with O(1) access time
// Works by splitting each number into a lower and upper part
// the lower part is left untouched while the upper part is
// compressed by a unary array in a quite elegant way :)

typedef struct{
    u64 n;
    bitarray * upper;
    select1c * S1;
    varray * lower;
    u64 lower_bits;
    u64 mem_allocated;
} cindex;

cindex * cindex_new(const u64 * A, u64 n);
u64 cindex_get(const cindex * C, u64 kk);
void cindex_free(cindex * C);
