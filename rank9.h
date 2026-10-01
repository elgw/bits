#pragma once

#include <stdint.h>
#include <stdlib.h>

typedef uint64_t u64;
typedef uint8_t u8;

typedef struct {
    u64 rankp; // 1 64-bit number
    u64 seven; // 7 9-bit numbers
} rank9_bin;

typedef struct {
    rank9_bin * bin;
    u64 * bits;
    u64 nbit;
} rank9;

// nbit needs to be a multiple of 64*8 = 512
rank9 * rank9_init(u64 * bits, u64 nbit);

// rank_1(r9, b)
u64 rank9_get(const rank9 * r9, u64 b);

void rank9_free(rank9 * r9);

void rank9_print(const rank9 * r9);

// should be the more cache friendly version
// faster then a dense array around at around 1M elements

typedef struct {
    u64 rankp; // 1 64-bit number
    u64 seven; // 7 9-bit numbers
    u64 bits[8]; // a 512 bit chunk copied from the bit array
} rank9b_bin;

typedef struct {
    rank9b_bin * bin;
    u64 nbit;
} rank9b;

// nbit needs to be a multiple of 64*8 = 512
rank9b * rank9b_init(u64 * bits, u64 nbit);

// rank_1(r9, b)
u64 rank9b_get(const rank9b * r9, u64 b);

void rank9b_free(rank9b * r9);

void rank9b_print(const rank9b * r9);

// Select

// One for every k1 of the arguments (i.e. one for each k1 ones).
// for rank9 there was one for every 512 ..., i.e.
// the additional storage was O(64/512*n)
// here I guess that we can use a little more memory.
// unfortunately we will have to make at least one jump
// within the structure for each lookup if we have to jump to a sparse table.
// We have a different amount of memory available (to make it succint) per
// bin, with an upper limit of rank(pos+k1)-rank(pos)
//
// And please benchmark against a binary search over rank9!
typedef struct {
  // which **bit** in B that represents select9(a)
  u64 pos;
  // we can use the highest bit to store the bin type.
  u8 bin_type; // dense or sparse.
  // the size of the region is given by
  // bin[k+1].pos - pos
  // if the range spans more than 32 bits per argument
  // then we can afford to store a lookup table.
  
  // If there are 
  
  // if the bin is dense, we just need some offsets,
  // and then need to perform a linear scan from the offsets
  // since we could use a linear scan (up to a specific number of u64s)
  // we can store offsets for every k2 argument.
  
} select9_bin;

typedef struct {
  select9_bin * bin;
  u64 nbit;
}  select9 ;
