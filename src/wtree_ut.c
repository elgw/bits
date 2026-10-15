#include <assert.h>
#include <stdio.h>

#include "wtree.h"

// Define some alphabet with 4 characters
const char c0 = 'A';
const char c1 = 'C';
const char c2 = 'G';
const char c3 = 'T';
char b2c[4] = {c0, c1, c2, c3};
static u64 c2b(char c){
    switch(c){
    case c0:
        return 0;
    case c1:
        return 1;
    case c2:
        return 2;
    case c3:
        return 3;
    }
    assert(0);
    return 0;
}

// Convert an array of nucleotides into a 2-bit vector
// TODO: Add as special case to varray?
u64 * nt_to_bin(char * seq, u64 nchar){
    u64 nword = nchar / 32;
    assert(nchar % 32 == 0);
    u64 * words = calloc(nword, sizeof(u64));

    for(u64 kk = 0; kk < nword; kk++){
        for(u64 ll = 0; ll < 32; ll++){
            u64 c = c2b(seq[kk*32 + ll]);
            words[kk] |= (c << (2*ll));
            //printf("ll=%lu c=%lu/%c:  0x%016lx\n", ll, c, b2c[c], words[kk]);
        }
    }

    return words;
}

// Convert from 2-bit array back to nucleotide array
char * bin_to_nt(u64 * words, u64 n_word){
    u64 nchar = 32*n_word;
    assert(nchar % 32 == 0);
    char * seq = calloc(nchar, sizeof(char));

    for(u64 kk = 0; kk < n_word; kk++){
        for(u64 ll = 0; ll < 32; ll++){
            seq[32*kk+ll] = b2c[(words[kk] >> (2*ll)) % 4];
            assert(seq[32*kk+ll] <= 't');
        }
    }
    return seq;
}

// generate a sequence of random nucleotides
char * randseq(u64 n){
    assert(n%32 == 0);
    char * S = malloc(n*sizeof(char));
    for(u64 kk = 0; kk < n; kk++){
        S[kk] = b2c[rand() % 4];
    }
    return S;
}

void wtree_ut(int verbose)
{
    if(verbose > 0){
        printf("%s\n", __FILE__);
    }

    // Create a dummy sequence/string
    u64 nchar = 256;
    char * seq = randseq(nchar);
    if(verbose > 1){
        for(u64 kk = 0; kk < nchar; kk++){
            if((kk > 0) && ((kk%64) == 0)){ printf(",");}
            printf("%c", seq[kk]);
        } printf("\n");
    }
    // Put the string it in a sequence of u64
    u64 * words = nt_to_bin(seq, nchar);
    // Convert back and check that the inverse also works
    char * seq2 = bin_to_nt(words, nchar/32);
    for(u64 kk = 0; kk < nchar; kk++){
        assert(seq[kk] == seq2[kk]);
    }
    free(seq2);


    wtree2 * W = wtree2_new(words, nchar);

    if(verbose > 0){
        printf("wtree2_access\n");
    }
    for(u64 kk = 0; kk < nchar; kk++){
        u64 v = wtree2_access(W, kk);
        assert(v < 4);
        char c = b2c[v];
        assert(c == seq[kk]);
    }
    if(verbose > 0){
        printf("wtree2_select1 (TODO)\n");
    }

    // create a reference implementation that linearly scans the nucleotide array

    if(verbose > 0){
        printf("wtree2_rank1 (TODO)\n");
    }

    // create a reference implementation that linearly scans the nucleotide array

    free(seq);
    wtree2_free(W);

    free(words);
    return;
}
