Warning, throwaway code ahead! This repo provides some functions for
working on bit arrays and also a few succint data
structures.

Implemented for fun and learning (while reading [^4]). Neither elegant
nor the fastest in the world, and with approximately one bug per
line. If you need these things, check out something more mature like
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

<details><summary>Timings</summary>

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

<details><summary>Linear scanning</summary>

For reference, this is how long time it takes to scan for a bit set at
increasing distance from the beginning of an array, using
`select1_raw` in `bits.c`.

Turns out that `_pdep` being extremely slow on Ryzen 3700X which is
very unfortunate for bit manipulations and
[chess](https://talkchess.com/viewtopic.php?t=72538). Even software
implementations are faster. In the table below there are timings using
the pded instruction as well as for my quickselect-like algorithm
(`bitpos.h`).

`./bits --benchmark 7`

|    query |    pdep | software |
|---------:|--------:|---------:|
|        1 |     396 |      108 |
|        2 |     360 |      108 |
|        4 |     360 |       72 |
|        8 |     324 |       72 |
|       16 |     360 |      108 |
|       32 |     360 |      108 |
|       64 |     360 |       72 |
|      128 |     360 |      108 |
|      256 |     396 |      144 |
|      512 |     396 |      180 |
|     1024 |     540 |      144 |
|     2048 |     504 |      180 |
|     4096 |     504 |      468 |
|     8192 |     612 |      288 |
|    16384 |     756 |      504 |
|    32768 |    1548 |     1116 |
|    65536 |    2736 |     2160 |
|   131072 |    7704 |     4536 |
|   262144 |    9216 |     8208 |
|   524288 |   21492 |    17784 |
|  1048576 |   36540 |    29268 |
|  2097152 |   73692 |    58392 |
|  4194304 |  163692 |   118692 |
|  8388608 |  315900 |   251532 |
| 16777216 |  485316 |   533448 |
| 33554432 |  934164 |  1038636 |
| 67108864 | 1856952 |  1975212 |

</details>

<details><summary>Timings</summary>

Please see the note on linear scanning above. Performance clouded by
slow pded on my machine.

``` nushell
$ make OPT=1 -B
$ ./bits --benchmark 2
```

The **array** in this case is a lookup-table, using 64x the memory of the
binary array.

Reporting average rdts time for selecting a random 1 in an array
with approximately 50% density. Random access pattern.

|             N | T_select1 | T_select1c | T_select1d | T_array |
|--------------:|----------:|-----------:|-----------:|--------:|
|           512 |       144 |        108 |        109 |      69 |
|         1,024 |       154 |        110 |        114 |      70 |
|         2,048 |       171 |        112 |        128 |      70 |
|         4,096 |       172 |        117 |        150 |      70 |
|         8,192 |       168 |        112 |        159 |      68 |
|        16,384 |       156 |        102 |        153 |      63 |
|        32,768 |       144 |         93 |        142 |      58 |
|        65,536 |       144 |         92 |        142 |      60 |
|       131,072 |       145 |         91 |        142 |      64 |
|       262,144 |       148 |         91 |        143 |      67 |
|       524,288 |       151 |         92 |        143 |      70 |
|     1,048,576 |       154 |         94 |        144 |      78 |
|     2,097,152 |       161 |         96 |        146 |     181 |
|     4,194,304 |       174 |        100 |        151 |     318 |
|     8,388,608 |       191 |        109 |        159 |     381 |
|    16,777,216 |       203 |        115 |        163 |     405 |
|    33,554,432 |       232 |        120 |        171 |     424 |
|    67,108,864 |       329 |        135 |        199 |     471 |
|   134,217,728 |       523 |        211 |        298 |     572 |
|   268,435,456 |       676 |        324 |        402 |     653 |
|   536,870,912 |       766 |        383 |        458 |     690 |
| 1,073,741,824 |       831 |        427 |        503 |     717 |
| 2,147,483,648 |       886 |        438 |        515 |     730 |
| 4,294,967,296 |       890 |        439 |        515 |     730 |


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
|           512 |      162 |      69 |         0.12 |
|         1,024 |      175 |      69 |         0.10 |
|         2,048 |      177 |      69 |         0.09 |
|         4,096 |      176 |      69 |         0.08 |
|         8,192 |      177 |      69 |         0.08 |
|        16,384 |      177 |      69 |         0.08 |
|        32,768 |      177 |      70 |         0.08 |
|        65,536 |      179 |      71 |         0.08 |
|       131,072 |      181 |      75 |         0.08 |
|       262,144 |      184 |      78 |         0.08 |
|       524,288 |      189 |      80 |         0.08 |
|     1,048,576 |      196 |      87 |         0.08 |
|     2,097,152 |      194 |     160 |         0.08 |
|     4,194,304 |      198 |     288 |         0.08 |
|     8,388,608 |      268 |     364 |         0.08 |
|    16,777,216 |      360 |     397 |         0.08 |
|    33,554,432 |      462 |     422 |         0.08 |
|    67,108,864 |      549 |     469 |         0.08 |
|   134,217,728 |      653 |     565 |         0.08 |
|   268,435,456 |      783 |     654 |         0.08 |
|   536,870,912 |      857 |     695 |         0.08 |
| 1,073,741,824 |      923 |     715 |         0.08 |

</details>

### **elias_codes**
Forward and backward transforms for the $`\gamma`$ and $`\delta`$ self
delimiting Elias codes. Also routines for encoding and decoding arrays
(but no random access!).

$`\gamma`$ encoded numbers use $`2\left\lceil\log_2(x+1)\right\rceil-1`$ bits.

$`\delta`$ encoded numbers use $`2\left\lceil\log_2(\left\lceil\log_2(x+1)\right\rceil+1)\right\rceil+\left\lceil\log2 (x+1)\right\rceil -1`$ bits.

Typical applications would be to encode the differences between
elements in sorted array of unique numbers.

<details><summary>Timings</summary>

The time needed for encoding/decoding routines grows with the size of
the numbers.

```
$ stress -c 8 -t 8 ; ./bits --benchmark 6
stress: info: [3406134] dispatching hogs: 8 cpu, 0 io, 0 vm, 0 hdd
stress: info: [3406134] successful run completed in 8s
Elias gamma, encode 1, ..., 1000000
   Took 6352632, or 6.4 per element
Elias gamma, encode and decode 1, ..., 1000000
   Took 14812200, or 14.8 per element
Elias delta, encode 1, ..., 1000000
   Took 7089732, or 7.1 per element
Elias delta, encode and decode 1, ..., 1000000
   Took 13559940, or 13.6 per element
Elias gamma array, encoding 1, ... 1000000
   Encoded 1000000 numbers using 36902912 bits (as u64 64000000 bits)
   Took 23843376, or 23.8 per element
   Decoded 1000000 numbers
   Took 11515176, or 11.5 per element
Elias delta array, encoding 1, ... 1000000
   Encoded 1000000 numbers using 27885696 bits (as u64 64000000 bits)
   Took 24985548, or 25.0 per element
   Decoded 1000000 numbers
   Took 17681004, or 17.7 per element
```

</details>

## Wavelet tree

For alphabet sizes of 4 (with ACGT in mind).

- access, implemented but quite uninteresting since a 2-bit array will
  always be faster (the wavelet tree spreads the data so there will be
  two cache misses per access using an alphabet of four).
- selectC -- todo
- rankC -- todo

<details><summary>timings of access</summary>

`make OPT=1 &&./bits --benchmark 8`

Time to access random elements in arrays of growing size. The results
are dominated by memory latencies for large "genomes".

Here compared to an array of u8`s to at least show something good
  (there is a range of problem sizes where the wavelet tree provides
  faster access).


|          N | u8 array | wtree2 |
|-----------:|---------:|-------:|
|       4096 |     69.3 |   88.4 |
|       8192 |     69.3 |   88.2 |
|      16384 |     69.4 |   88.5 |
|      32768 |     69.5 |   88.4 |
|      65536 |     69.4 |   88.3 |
|     131072 |     69.5 |   88.8 |
|     262144 |     69.7 |   90.7 |
|     524288 |     69.5 |   95.0 |
|    1048576 |     71.1 |   97.4 |
|    2097152 |     74.4 |  104.0 |
|    4194304 |     77.4 |  121.5 |
|    8388608 |     77.6 |  143.6 |
|   16777216 |     79.1 |  149.6 |
|   33554432 |    127.9 |  138.7 |
|   67108864 |    242.2 |  149.3 |
|  134217728 |    315.1 |  293.8 |
|  268435456 |    356.3 |  718.6 |
|  536870912 |    378.7 |  701.8 |
| 1073741824 |    393.3 |  783.5 |
| 2147483648 |    406.5 |  822.6 |
| 4294967296 |    429.1 |  855.3 |

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

- Intel documentation about [PEXT](https://intel.github.io/SDM/instruction/PEXT.html)

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

[^4]: V. Mäkinen, D. Belazzougui, F. Cunial, and A. I. Tomescu, Genome-Scale Algorithm Design: Bioinformatics in the Era of High-Throughput Sequencing, 2nd ed. Cambridge: Cambridge University Press, 2023. [doi:10.1017/9781009341257](https://doi.org/10.1017/9781009341257)
