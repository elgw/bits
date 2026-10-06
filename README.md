## Building

-  Build on GCC 13.3.0 and clang 18.1.3 under x86_64-pc-linux-gnu.

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

### **select1**
- construction in $`\mathcal{O}(n)`$ time, using $`\mathcal{O}(n/16)`$
bits extra memory.

<details><summary>Timings</summary>

``` nushell
$ make OPT=1 -B
$ ./bits --benchmark 2
```

Reporting average rdts time for selecting a random 1 in an array
with approximately 50% density.

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
|    16,777,216 |       295 |     329 |
|    33,554,432 |       314 |     387 |
|    67,108,864 |       363 |     413 |
|   134,217,728 |       502 |     433 |
|   268,435,456 |       668 |     481 |
|   536,870,912 |       846 |     589 |
| 1,073,741,824 |       923 |     640 |
| 2,147,483,648 |     1,048 |     705 |
| 4,294,967,296 |     1,184 |     970 |
| 8,589,934,592 |     1,169 |     746 |

</summary>

### **cindex**
Elias-Fano representation of non-decreasing sequences.
- Construction.

## Notes/links

- [Time Stamp
Counter](https://en.wikipedia.org/wiki/Time_Stamp_Counter) used for
the timings via the `__rdtscp` intrinsics.

- [Sean Eron Anderson's Bit Twiddling Hacks](https://graphics.stanford.edu/~seander/bithacks.html)

- For see [Builtin Bit Operations in GCC](https://gcc.gnu.org/onlinedocs/gcc/Bit-Operation-Builtins.html)

- `x86intrin.h`

- Can `__builtin_stdc_rotate_left` be used to zero out? `<<` can't be
used for a full shift. I think that could be used for select9.

[^1]: Vigna, S. (2008). Broadword Implementation of Rank/Select
Queries. In: McGeoch, C.C. (eds) Experimental
Algorithms. WEA 2008. Lecture Notes in Computer Science,
vol 5038. Springer, Berlin,
Heidelberg. [https://doi.org/10.1007/978-3-540-68552-4_12] Also
available from
[https://vigna.di.unimi.it/ftp/papers/Broadword.pdf] Code (rust)
can be found at [https://github.com/vigna/sux-rs]
