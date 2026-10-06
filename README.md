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

|             N | T_select1 | T_array |
|--------------:|----------:|--------:|
|           512 |       247 |      70 |
|         1,024 |       254 |      71 |
|         2,048 |       281 |      71 |
|         4,096 |       305 |      71 |
|         8,192 |       306 |      71 |
|        16,384 |       300 |      71 |
|        32,768 |       304 |      71 |
|        65,536 |       308 |      73 |
|       131,072 |       309 |      77 |
|       262,144 |       304 |      80 |
|       524,288 |       310 |      85 |
|     1,048,576 |       310 |      87 |
|     2,097,152 |       316 |     115 |
|     4,194,304 |       311 |     248 |
|     8,388,608 |       287 |     346 |
|    16,777,216 |       296 |     372 |
|    33,554,432 |       313 |     406 |
|    67,108,864 |       356 |     423 |
|   134,217,728 |       500 |     475 |
|   268,435,456 |       735 |     581 |
|   536,870,912 |       831 |     650 |
| 1,073,741,824 |     1,022 |     734 |
| 2,147,483,648 |     1,009 |     716 |

</details>

### **cindex** (TODO)
Elias-Fano representation of non-decreasing sequences.
- Construction.

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

[^1]: Vigna, S. (2008). Broadword Implementation of Rank/Select
Queries. In: McGeoch, C.C. (eds) Experimental
Algorithms. WEA 2008. Lecture Notes in Computer Science,
vol 5038. Springer, Berlin,
Heidelberg. [https://doi.org/10.1007/978-3-540-68552-4_12] Also
available from
[https://vigna.di.unimi.it/ftp/papers/Broadword.pdf] Code (rust)
can be found at [https://github.com/vigna/sux-rs]
