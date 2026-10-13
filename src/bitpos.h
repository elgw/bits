#pragma once
#include <stdint.h>

// software emulation of pdep functionality for systems where that is
// slow (Zen 1 / Zen+ / Zen 2)
//
// Return the 0-based position of the ith 1 of the word
// i.e., select1_word(0xffffffffffffffff, kk) -> kk, kk \in [0, 63]
// returns 63 if there are less than i 1s
// i.e. the rightmost positions.


#ifdef NOPDEP
static uint64_t bitpos(const uint64_t w, uint64_t i)
{
    assert((uint64_t) __builtin_popcountl(w) >= i);
    uint64_t pos = 0;
    uint64_t mask = 0x00000000ffffffff;
    uint64_t width = 32; // number of bits in mask
    for(int kk = 0; kk < 6; kk++){
        #if 0
        uint64_t t = mask;
        for(int ll = 0; ll < 64; ll++){
            printf("%lu", t % 2);
            t /= 2;
        }
        printf("\n");
        #endif
        assert(mask != 0);
        uint64_t a = (uint64_t)  __builtin_popcountl(w & mask);
        if(a > i) { // stay left
            mask = mask & (mask >> (width/2));
        } else {
            i -= a;
            pos += width;
            mask = mask << width;
            mask = mask & mask >> (width/2);
        }
        width /= 2;
    }
    return pos;
}
#else
static uint64_t bitpos(const uint64_t w, uint64_t i)
{
    return __builtin_ctzl(_pdep_u64(1LU << (i-1), w));
}
#endif
