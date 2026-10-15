#include <assert.h>
#include <stdio.h>

#include "wtree.h"

char b2c[4] = {'a', 'c', 'g', 't'};
static u64 c2b(char c){
    switch(c){
    case 'a':
        return 0;
    case 'c':
        return 1;
    case 'g':
        return 2;
    case 't':
        return 3;
    }
    assert(0);
    return 0;
}

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
        printf("0x%016lx", words[kk]);
        if(kk + 1 == nword){
            printf("\n");
        } else {
            printf(", ");
        }
    }

    return words;
}

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
    u64 nchar = 256;
    char * seq = randseq(nchar);
    for(u64 kk = 0; kk < nchar; kk++){
        if(kk > 0 & (kk%64) == 0){ printf(",");}
        printf("%c", seq[kk]);
    } printf("\n");
    u64 * words = nt_to_bin(seq, nchar);
    char * seq2 = bin_to_nt(words, nchar/32);
    for(u64 kk = 0; kk < nchar; kk++){
        assert(seq[kk] == seq2[kk]);
    }
    free(seq2);
    free(seq);

    wtree * W = wtree_new(words, nchar, 2);

    // TODO: Test access

    // TODO: Test select1

    // TODO: Test rank1

    wtree_free(W);

    free(words);
    return;
}
