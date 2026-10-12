#pragma once

#include <stdint.h>

// Elias codes
// If L2(x) = ceil(log2(x))
//
// gamma codes require, 2*L2(x) - 1 bits to encode x
// [ L2(x)-1 0s, bin(x) ]
// gamma(5) = 00 101 (5 bits)
//            |   ﾤ  bin(5)
//            L2(5)-1 = 2 0's
//
// gamma(77) = 000000 1001101 (13 bits)
//             |        ﾤ  bin(77)
//            L2(77)-6 = 2 0's
//
//
// delta codes requires 2*L2(L2(x)) + L2(x) - 1 bits
// [ L2(L2) 0s , bin(l2(x)), x ]
// delta(5) = 0 11 101 (6 bits)
//            | |  ﾤ  bin(5)
//            | ﾤ bin(L2(5))
//            L2(L2(5)) - 1 0s
//
// delta(77) = 00 111 1001101 (12 bits)
//             |  |   ﾤ  bin(77)
//             |  ﾤ bin(L2(77))
//             L2(L2(77)) - 1 0s


// return the number of bits needed to encode n
uint64_t egamma_bits(uint64_t n);
uint64_t egamma_encode(uint64_t decoded);
uint64_t egamma_decode(uint64_t encoded);

// Encode n_decoded numbers and store in target
// containing n_encoded uint64_t
// return the number of values that could be encoded
// in the target space
uint64_t
egamma_encode_array(const uint64_t * decoded, uint64_t n_decoded,
                             uint64_t * encoded, uint64_t n_encoded);

uint64_t
egamma_decode_array(const uint64_t * encoded, uint64_t n_encoded,
                             uint64_t * decoded, uint64_t n_decoded);


// return the number of bits needed to encode n
uint64_t edelta_bits(uint64_t n);
uint64_t edelta_encode(uint64_t decoded);
uint64_t edelta_decode(uint64_t decoded);

void elias_codes_ut(int verbose);

uint64_t
edelta_encode_array(const uint64_t * decoded, uint64_t n_decoded,
                    uint64_t * encoded, uint64_t n_encoded);

uint64_t
edelta_decode_array(const uint64_t * encoded, uint64_t n_encoded,
                    uint64_t * decoded, uint64_t n_decoded);
