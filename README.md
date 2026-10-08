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

<details><summary>Timings</summaray>

```
$ ./bits --benchmark 5
```

Reporting the time it takes to read an element at a random position.

|             N | T_bitarray | T_u8 |
|--------------:|-----------:|-----:|
|           128 |         37 |   34 |
|           256 |         36 |   33 |
|           512 |         36 |   33 |
|         1,024 |         36 |   33 |
|         2,048 |         36 |   33 |
|         4,096 |         36 |   33 |
|         8,192 |         36 |   33 |
|        16,384 |         36 |   33 |
|        32,768 |         36 |   33 |
|        65,536 |         37 |   34 |
|       131,072 |         37 |   35 |
|       262,144 |         39 |   41 |
|       524,288 |         41 |   52 |
|     1,048,576 |         48 |   58 |
|     2,097,152 |         56 |   61 |
|     4,194,304 |         63 |   68 |
|     8,388,608 |         70 |  174 |
|    16,777,216 |         91 |  278 |
|    33,554,432 |        151 |  328 |
|    67,108,864 |        252 |  355 |
|   134,217,728 |        326 |  376 |
|   268,435,456 |        339 |  379 |
|   536,870,912 |        361 |  386 |
| 1,073,741,824 |        377 |  424 |
| 2,147,483,648 |        385 |  525 |

</details>

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

|             N | T_select1 | T_select1c | T_select1d | T_array |
|--------------:|----------:|-----------:|-----------:|--------:|
|           512 |        98 |         73 |         83 |      43 |
|         1,024 |        82 |         61 |         63 |      33 |
|         2,048 |        99 |         66 |         63 |      33 |
|         4,096 |       103 |         75 |         64 |      35 |
|         8,192 |       102 |         72 |         63 |      38 |
|        16,384 |       101 |         70 |         63 |      36 |
|        32,768 |       101 |         68 |         63 |      39 |
|        65,536 |       101 |         68 |         63 |      48 |
|       131,072 |       104 |         68 |         64 |      57 |
|       262,144 |       104 |         68 |         64 |      63 |
|       524,288 |       109 |         71 |         66 |      86 |
|     1,048,576 |       118 |         74 |         68 |     201 |
|     2,097,152 |       138 |         81 |         74 |     331 |
|     4,194,304 |       155 |         88 |         85 |     374 |
|     8,388,608 |       173 |         96 |         93 |     390 |
|    16,777,216 |       198 |        103 |        100 |     404 |
|    33,554,432 |       274 |        109 |        116 |     425 |
|    67,108,864 |       428 |        141 |        187 |     501 |
|   134,217,728 |       621 |        264 |        295 |     601 |
|   268,435,456 |       685 |        323 |        343 |     632 |
|   536,870,912 |       752 |        368 |        380 |     656 |
| 1,073,741,824 |       791 |        386 |        398 |     667 |
| 2,147,483,648 |       802 |        398 |        408 |     670 |
| 4,294,967,296 |       833 |        402 |        410 |     672 |

select1d is incomplete, but might be the basis for the next version.

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

- Things are designed to work with u64 but could of course be more
  memory efficient if u32 is the design case ... simpler to do in a
  language with templates or comptime abilities.

- Timings typically depends on the density of 1's as well as the
  access pattern. Small test sizes gives the overhead of the bit
  manipulations while the large sizes reveals cache misses.

- Benchmarks are not consistently run on the same machine.

- Where the benchmarks compare to arrays, those are represented by
  `u64` words..

- Timings are done by reading the [Time Stamp
Counter](https://en.wikipedia.org/wiki/Time_Stamp_Counter) via the
`__rdtscp` intrinsics.

- Some fun things can be found here: [Sean Eron Anderson's Bit
  Twiddling
  Hacks](https://graphics.stanford.edu/~seander/bithacks.html), most
  likely the compiler can figure out some of them even without explicit code.

- It is not always necessary to write asm code, see [Builtin Bit
  Operations in
  GCC](https://gcc.gnu.org/onlinedocs/gcc/Bit-Operation-Builtins.html)
  as well as your local copy of `x86intrin.h`

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
