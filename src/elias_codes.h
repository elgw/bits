#pragma once

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

typedef uint64_t u64;

// Encode number and store the result in bits, starting at the bit start
// return the number of bits needed
u64 encode_gamma(u64 decoded);
u64 decode_gamma(u64 encoded);

u64 encode_delta(u64 decoded);
u64 decode_delta(u64 decoded);

void elias_codes_ut(void);
