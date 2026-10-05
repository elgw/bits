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

### **cindex**
Elias-Fano representation of non-decreasing sequences.
- Construction.

## Notes/links

- For see [Builtin Bit Operations in GCC](https://gcc.gnu.org/onlinedocs/gcc/Bit-Operation-Builtins.html)

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
