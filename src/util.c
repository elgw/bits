#include <stdio.h>
#include "util.h"

void print_binary(u64 x, u64 _nb){
    int nb = (int) _nb;
    while(nb > 0)
    {
        if(nb == 32){
            printf(" ");
        }
        printf("%ld", x % 2);
        x/=2;
        nb--;
    }
}

void print_binaryl(const char * s, u64 x){
    printf("%s: ", s);
    print_binary(x, 64);
    printf("\n");
}
