#include <assert.h>
#include <stdio.h>

#include "wtree.h"
#include "bitarray.h"

// Define some alphabet with 4 characters
const char Sigma[4] = {'a', 'c', 'g', 'T'};
static u64 iSigma(char c){
    switch(c){
    case Sigma[0]:
        return 0;
    case Sigma[1]:
        return 1;
    case Sigma[2]:
        return 2;
    case Sigma[3]:
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
            u64 c = iSigma(seq[kk*32 + ll]);
            words[kk] |= (c << (2*ll));
            //printf("ll=%lu c=%lu/%c:  0x%016lx\n", ll, c, Sigma[c], words[kk]);
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
            seq[32*kk+ll] = Sigma[(words[kk] >> (2*ll)) % 4];
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
        S[kk] = Sigma[rand() % 4];
    }
    return S;
}

u64 rankc_w2(u64 c, u64 * data, u64 n){
    u64 test = 1LU;
    bitarray * B0 = bitarray_new_from_data(&test, 64);
    assert(bitarray_get(B0, 0) == 1);
    assert(bitarray_rank1(B0, 0) == 1);
    B0->data = NULL;
    bitarray_free(B0);

    // count all characters up to and including n
    bitarray * B = bitarray_new_from_data(data, 2+2*n);
    u64 H[4] = {0};
    for(u64 kk = 0; kk <= n; kk++){
        u64 v = bitarray_get(B, 2*kk) + 2*bitarray_get(B, 2*kk+1);
        H[v]++;
    }
    u64 sum = H[0] + H[1] + H[2] + H[3];
    assert(sum == n + 1);

    B->data = NULL;
    bitarray_free(B);
    return H[c];
}

u64 selectc_w2(u64 c, u64 * data, u64 n, u64 k){
    u64 test = 1LU;
    bitarray * B0 = bitarray_new_from_data(&test, 64);
    assert(bitarray_get(B0, 0) == 1);
    assert(bitarray_select1(B0, 1) == 0);
    B0->data = NULL;
    bitarray_free(B0);

    assert(k > 0); // 1-indexing for some reason

    // count all characters up to and including n
    bitarray * B = bitarray_new_from_data(data, 2+2*n);
    u64 H[4] = {0};
    for(u64 kk = 0; kk < n; kk++){
        u64 v = bitarray_get(B, 2*kk) + 2*bitarray_get(B, 2*kk+1);
        H[v]++;
        if(H[c] == k){
            return kk;
        }
    }
    #if 0
    printf("Warning selectc_w2(%c, %lu), could not find withing bounds\n",
           Sigma[c], k);
    #endif
    return n;
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
        char c = Sigma[v];
        assert(c == seq[kk]);
    }

    if(verbose > 0){
        printf("wtree2_rankc\n");
    }
    if(verbose > 1){
        for(i64 kk = 0; kk < 16; kk++){
            printf("%c", seq[kk]);
        }
        printf("\n");
    }

    u64 nwrong = 0;
    for(u64 kk = 0; kk+1 < nchar; kk++){
        for(u64 ch = 0; ch < 4; ch++){
            u64 ref = rankc_w2(ch, words, kk);
            if(0){
                printf("rank_%c(%lu) = %lu\n", Sigma[ch], kk, ref);
            }
            u64 rc = wtree2_rankc(W, ch, kk);
            if(rc != ref){
                nwrong++;
                printf("ch=%c,kk=%lu, REF: %lu GOT: %lu\n", Sigma[ch], kk, ref, rc);
            }
            if(nwrong > 8){
                exit(EXIT_FAILURE);
            }
        }
    }

    if(verbose > 0){
        printf("wtree2_selectc (only for c = 3)\n");
    }

    if(verbose > 1){
        for(i64 kk = 0; kk < 16; kk++){
            printf("%c", seq[kk]);
        }
        printf("\n");
    }

    nwrong = 0;
    for(u64 kk = 1; kk < nchar/4; kk++){
        for(u64 ch = 3; ch < 4; ch++){
            u64 ref = selectc_w2(ch, words, nchar, kk);
            u64 rc = wtree2_selectc(W, ch, kk);
            if(ref < nchar){
                if(rc != ref){
                    nwrong++;
                    printf("ch=%c,kk=%lu, REF: %lu GOT: %lu\n", Sigma[ch], kk, ref, rc);
                }
            }
            if(nwrong > 8){
                exit(EXIT_FAILURE);
            }
        }
    }

    assert(nwrong == 0);
    free(seq);
    wtree2_free(W);

    free(words);
    return;
}
