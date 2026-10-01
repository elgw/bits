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

### **rank9**
- construction in $`\mathcal{O}(n)`$ time, using $`\mathcal{O}(n/4)`$
  bits extra memory.
- rank1, in $`\mathcal{O}(n)`$.
- select1_bs, in $`\mathcal{O}(\log n)`$ time.
  using binary search.

### **cindex**
Elias-Fano representation of non-decreasing sequences.
- Construction.

## Notes/links

- For see [Builtin Bit Operations in GCC](https://gcc.gnu.org/onlinedocs/gcc/Bit-Operation-Builtins.html)

- Can `__builtin_stdc_rotate_left` be used to zero out? `<<` can't be
  used for a full shift. I think that could be used for select9.
