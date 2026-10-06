Warning, throwaway code ahead! This repo provides some functions for
working on bit arrays and also a few succint data
structures. Implemented for fun and learning. If you need these
things, check out something more mature like
[sux-rs](https://github.com/vigna/sux-rs).


## Building

-  Build on GCC 13.3.0 and clang 18.1.3 under x86_64-pc-linux-gnu.

- Not portable. And select1 use `_pdep_u64` which is in the VEX.W1
  instruction set (part of AVX?).

For running benchmarks, use

``` shell
make OPT=1
```

For detecting memory leaks, use

```
make SAN=1 -B # Enables -fsanitize=address
./bits
```

or

```
make -B # -g3 and -Og
valgrind ./bits
```


## Contents

### **bitarray**
for manipulating binary arrays
- set and get is $`\mathcal{O}(1)`$
- rank1, in $`\mathcal{O}(n)`$ time but no memory overhead
- select1, in $`\mathcal{O}(n)`$ time but no memory overhead

### **varray**
variable bits per element array
- set
- get

### **rank1**
- construction in $`\mathcal{O}(n)`$ time, using $`\mathcal{O}(n/4)`$
bits extra memory.
- rank1, in $`\mathcal{O}(n)`$.
- select1_bs, in $`\mathcal{O}(\log n)`$ time.
using binary search.

- Inspired by Sebastiano Signa's rank9 [^1]

<details><summary>Timings</summary>

Reporting average rdts time for an array
with approximately 50% density. Random access pattern.


|             N | T_rank1 | T_rank1b | T_array |
|--------------:|--------:|---------:|--------:|
|           512 |      74 |       74 |      70 |
|         1,024 |      74 |       74 |      70 |
|         2,048 |      62 |       62 |      59 |
|         4,096 |      62 |       62 |      59 |
|         8,192 |      75 |       75 |      71 |
|        16,384 |      64 |       64 |      60 |
|        32,768 |      61 |       61 |      58 |
|        65,536 |      74 |       74 |      70 |
|       131,072 |      68 |       68 |      68 |
|       262,144 |      75 |       75 |      79 |
|       524,288 |      76 |       75 |      83 |
|     1,048,576 |      80 |       79 |      87 |
|     2,097,152 |      79 |       78 |     105 |
|     4,194,304 |      93 |       92 |     266 |
|     8,388,608 |      81 |       81 |     324 |
|    16,777,216 |     104 |      121 |     389 |
|    33,554,432 |     180 |      221 |     410 |
|    67,108,864 |     284 |      315 |     403 |
|   134,217,728 |     385 |      389 |     433 |
|   268,435,456 |     430 |      409 |     477 |
|   536,870,912 |     468 |      434 |     575 |
| 1,073,741,824 |     466 |      421 |     635 |
| 2,147,483,648 |     507 |      467 |     712 |

</details>

### **select1**
- construction in $`\mathcal{O}(n)`$ time, using $`\mathcal{O}(n/16)`$
bits extra memory. Using a single level of indirection. Will typically
trigger two cache misses.

<details><summary>Timings</summary>

``` nushell
$ make OPT=1 -B
$ ./bits --benchmark 2
```

Reporting average rdts time for selecting a random 1 in an array
with approximately 50% density. Random access pattern.

|             N | T_select1 | T_array |
|--------------:|----------:|--------:|
|           512 |       244 |      69 |
|         1,024 |       250 |      69 |
|         2,048 |       274 |      69 |
|         4,096 |       301 |      69 |
|         8,192 |       303 |      69 |
|        16,384 |       303 |      70 |
|        32,768 |       302 |      69 |
|        65,536 |       303 |      70 |
|       131,072 |       300 |      69 |
|       262,144 |       289 |      70 |
|       524,288 |       272 |      70 |
|     1,048,576 |       272 |     152 |
|     2,097,152 |       260 |     159 |
|     4,194,304 |       266 |     130 |
|     8,388,608 |       281 |     240 |
|    16,777,216 |   **295** |     329 |
|    33,554,432 |   **314** |     387 |
|    67,108,864 |   **363** |     413 |
|   134,217,728 |       502 |     433 |
|   268,435,456 |       668 |     481 |
|   536,870,912 |       846 |     589 |
| 1,073,741,824 |       923 |     640 |
| 2,147,483,648 |     1,048 |     705 |
| 4,294,967,296 |     1,184 |     970 |
| 8,589,934,592 |     1,169 |     746 |

</details>

### **cindex** (TODO)
Elias-Fano representation of non-decreasing sequences.
- Construction.

## Notes/links

- Timings typically depends on the density of 1's as well as the
  access pattern. Small test sizes gives the overhead of the bit
  manipulations while the large sizes reveals cache misses.

- [Time Stamp
Counter](https://en.wikipedia.org/wiki/Time_Stamp_Counter) used for
the timings via the `__rdtscp` intrinsics.

- [Sean Eron Anderson's Bit Twiddling Hacks](https://graphics.stanford.edu/~seander/bithacks.html)

- For see [Builtin Bit Operations in GCC](https://gcc.gnu.org/onlinedocs/gcc/Bit-Operation-Builtins.html)

- `x86intrin.h`

[^1]: Vigna, S. (2008). Broadword Implementation of Rank/Select
Queries. In: McGeoch, C.C. (eds) Experimental
Algorithms. WEA 2008. Lecture Notes in Computer Science,
vol 5038. Springer, Berlin,
Heidelberg. [https://doi.org/10.1007/978-3-540-68552-4_12] Also
available from
[https://vigna.di.unimi.it/ftp/papers/Broadword.pdf] Code (rust)
can be found at [https://github.com/vigna/sux-rs]
