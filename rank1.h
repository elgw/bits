#pragma once

// rank1 data structure
//

#include <stdint.h>
#include <stdlib.h>

typedef uint64_t u64;
typedef uint8_t u8;

// 128 bits indexes 512 bits of the binary array
// what is tabulated is the **exclusive prefix sums**
// i.e. rank1->bins[b].rankp is rank1(512*b-1).
// this choice requires 1 extra rank1_bin
typedef struct {
    u64 rankp; // 1 64-bit number
    u64 seven; // 7 9-bit numbers
} rank1_bin;

typedef struct {
    rank1_bin * bin;
    u64 * bits; // owned by the caller and not freed after use
    u64 nbit;
    u64 n_one; // Number of 1's
} rank1;

// nbit needs to be a multiple of 64*8 = 512
rank1 * rank1_init(u64 * bits, u64 nbit);

// rank_1(r1, b)
u64 rank1_get(const rank1 * r1, u64 b);

// select1 implemented by binary search
// returns non-zero if b == 0 or b > r1->n_ones
int rank1_select1_bs(const rank1 *, u64 b, u64 * select1);

void rank1_free(rank1 * r1);

void rank1_print(const rank1 * r1);


// should be the more cache friendly version
// faster then a dense array around at around 1M elements

typedef struct {
    u64 rankp; // 1 64-bit number
    u64 seven; // 7 9-bit numbers
    u64 bits[8]; // a 512 bit chunk copied from the bit array
} rank1b_bin;

typedef struct {
    rank1b_bin * bin;
    u64 nbit;
} rank1b;

// nbit needs to be a multiple of 64*8 = 512
rank1b * rank1b_init(u64 * bits, u64 nbit);

// rank_1(r1, b)
u64 rank1b_get(const rank1b * r1, u64 b);

void rank1b_free(rank1b * r1);

void rank1b_print(const rank1b * r1);

// Select

// One for every k1 of the arguments (i.e. one for each k1 ones).
// for rank1 there was one for every 512 ..., i.e.
// the additional storage was O(64/512*n)
// here I guess that we can use a little more memory.
// unfortunately we will have to make at least one jump
// within the structure for each lookup if we have to jump to a sparse table.
// We have a different amount of memory available (to make it succint) per
// bin, with an upper limit of rank(pos+k1)-rank(pos)
//
// And please benchmark against a binary search over rank1!
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
