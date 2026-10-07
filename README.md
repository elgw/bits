Warning, throwaway code ahead! This repo provides some functions for
working on bit arrays and also a few succint data
structures. Implemented for fun and learning. Not elegant, not fastest
in the world, probably at least one bug per line. If you need these
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

<details><summary>Timings</summary>

Testing the time it takes to access random number $`\leq 2^{33}`$ at
random positions. In this case varray is set to be 33 bits while the
"conventional" array uses 64 bits.

``` shell
./bits --benchmark 4
```

|           N | T_varray_u31 | T_array_u64 |
|------------:|-------------:|------------:|
|         512 |           78 |          72 |
|       1,024 |           78 |          71 |
|       2,048 |           78 |          72 |
|       4,096 |           79 |          72 |
|       8,192 |           78 |          71 |
|      16,384 |           77 |          71 |
|      32,768 |           79 |          73 |
|      65,536 |           85 |          76 |
|     131,072 |           93 |          80 |
|     262,144 |           99 |          82 |
|     524,288 |          105 |          91 |
|   1,048,576 |          122 |         132 |
|   2,097,152 |          229 |         285 |
|   4,194,304 |          329 |         359 |
|   8,388,608 |          347 |         364 |
|  16,777,216 |          393 |         393 |
|  33,554,432 |          401 |         399 |
|  67,108,864 |          415 |         418 |
| 134,217,728 |          421 |         433 |
| 268,435,456 |          462 |         515 |
| 536,870,912 |          577 |         638 |

</details>

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
|           512 |      73 |       73 |      69 |
|         1,024 |      65 |       65 |      61 |
|         2,048 |      72 |       72 |      69 |
|         4,096 |      73 |       73 |      69 |
|         8,192 |      61 |       61 |      58 |
|        16,384 |      62 |       62 |      58 |
|        32,768 |      62 |       62 |      59 |
|        65,536 |      69 |       69 |      68 |
|       131,072 |      74 |       74 |      78 |
|       262,144 |      63 |       63 |      69 |
|       524,288 |      66 |       66 |      76 |
|     1,048,576 |      81 |       80 |     120 |
|     2,097,152 |      77 |       76 |     211 |
|     4,194,304 |      93 |       93 |     331 |
|     8,388,608 |      96 |      100 |     399 |
|    16,777,216 |     102 |      122 |     407 |
|    33,554,432 |     199 |      241 |     430 |
|    67,108,864 |     319 |      346 |     456 |
|   134,217,728 |     422 |      413 |     539 |
|   268,435,456 |     450 |      423 |     606 |
|   536,870,912 |     463 |      426 |     653 |
| 1,073,741,824 |     483 |      440 |     700 |
| 2,147,483,648 |     513 |      473 |     739 |


</details>

### **select1**
- construction in $`\mathcal{O}(n)`$ time, using $`\mathcal{O}(n/16)`$
bits extra memory. Using a single level of indirection. Will typically
trigger two cache misses.

- Can be used to encode **increasing** sequences $`[s_0 \geq 0,
  s_1>s_0, ..., s_k <n]`$ in $`n\frac{17}{16}`$ bits, or even
  **non-decreasing** sequences, $`[s_0 \geq 0, s_1 \geq s_0, ..., s_k
  <n]`$ via a 1-1 mapping $`x\rightarrow x+1`$ in $`(k+n)\frac{17}{16}`$ bits.

<details><summary>Timings</summary>

``` nushell
$ make OPT=1 -B
$ ./bits --benchmark 2
```

Reporting average rdts time for selecting a random 1 in an array
with approximately 50% density. Random access pattern.

|             N | T_select1 | T_select1c | T_array |
|--------------:|----------:|-----------:|--------:|
|           512 |       398 |        385 |      70 |
|         1,024 |       402 |        388 |      70 |
|         2,048 |       414 |        387 |      70 |
|         4,096 |       413 |        394 |      70 |
|         8,192 |       394 |        373 |      66 |
|        16,384 |       353 |        335 |      60 |
|        32,768 |       437 |        408 |      74 |
|        65,536 |       413 |        386 |      75 |
|       131,072 |       414 |        386 |      82 |
|       262,144 |       419 |        388 |      89 |
|       524,288 |       417 |        382 |     103 |
|     1,048,576 |       385 |        350 |     149 |
|     2,097,152 |       359 |        322 |     215 |
|     4,194,304 |       371 |        324 |     309 |
|     8,388,608 |       383 |        333 |     367 |
|    16,777,216 |       398 |        339 |     407 |
|    33,554,432 |       413 |        343 |     427 |
|    67,108,864 |       486 |        350 |     447 |
|   134,217,728 |       663 |        393 |     532 |
|   268,435,456 |       811 |        486 |     624 |
|   536,870,912 |       940 |        578 |     680 |
| 1,073,741,824 |     1,015 |        628 |     708 |
| 2,147,483,648 |     1,096 |        678 |     740 |
| 4,294,967,296 |     1,077 |        662 |     727 |

</details>

### **cindex**
Elias-Fano representation [^2], [^3] of non-decreasing sequences.

- Using bitarray, varray and select1c from above.


<details><summary>Timings</summary>

``` shell
$ make -B OPT=1
$ ./bits --benchmark 3
```
|             N | T_cindex | T_array | cindex_mem_Q |
|--------------:|---------:|--------:|-------------:|
|           512 |      294 |      71 |         0.12 |
|         1,024 |      304 |      71 |         0.10 |
|         2,048 |      305 |      70 |         0.09 |
|         4,096 |      306 |      71 |         0.08 |
|         8,192 |      308 |      71 |         0.08 |
|        16,384 |      303 |      70 |         0.08 |
|        32,768 |      295 |      68 |         0.08 |
|        65,536 |      279 |      69 |         0.08 |
|       131,072 |      262 |      70 |         0.08 |
|       262,144 |      259 |      69 |         0.08 |
|       524,288 |      262 |      72 |         0.08 |
|     1,048,576 |      335 |     130 |         0.08 |
|     2,097,152 |      333 |     251 |         0.08 |
|     4,194,304 |      351 |     358 |         0.08 |
|     8,388,608 |      389 |     410 |         0.08 |
|    16,777,216 |      424 |     415 |         0.08 |
|    33,554,432 |      567 |     444 |         0.08 |
|    67,108,864 |      622 |     457 |         0.08 |
|   134,217,728 |      731 |     548 |         0.08 |
|   268,435,456 |      843 |     633 |         0.08 |
|   536,870,912 |      938 |     690 |         0.08 |
| 1,073,741,824 |    1,061 |     749 |         0.08 |

</details>

## Notes/links

- Timings typically depends on the density of 1's as well as the
  access pattern. Small test sizes gives the overhead of the bit
  manipulations while the large sizes reveals cache misses.

- Arrays, typically representing LUTs, are `u64`.

- [Time Stamp
Counter](https://en.wikipedia.org/wiki/Time_Stamp_Counter) used for
the timings via the `__rdtscp` intrinsics.

- [Sean Eron Anderson's Bit Twiddling Hacks](https://graphics.stanford.edu/~seander/bithacks.html)

- For see [Builtin Bit Operations in GCC](https://gcc.gnu.org/onlinedocs/gcc/Bit-Operation-Builtins.html)

- `x86intrin.h`

[^3]: [The Elias–Fano coding method by Giulio Ermanno
Pibiri](https://jermp.github.io/assets/pdf/notes/elias_fano_notes.pdf)

[^2]: [Sorted integers compression with Elias-Fano encoding by Antonio Mallia](https://www.antoniomallia.it/sorted-integers-compression-with-elias-fano-encoding.html)

[^1]: Vigna, S. (2008). Broadword Implementation of Rank/Select
Queries. In: McGeoch, C.C. (eds) Experimental
Algorithms. WEA 2008. Lecture Notes in Computer Science,
vol 5038. Springer, Berlin,\
Heidelberg. [https://doi.org/10.1007/978-3-540-68552-4_12] Also
available from
[https://vigna.di.unimi.it/ftp/papers/Broadword.pdf] Code (rust)
can be found at [https://github.com/vigna/sux-rs]
