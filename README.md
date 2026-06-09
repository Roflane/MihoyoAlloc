# Mihoyo::Alloc — reverse engineered memory allocator

Readable C++ reconstruction of a pool allocator 
recovered via reverse engineering from a Mihoyo game binary.

**Stack:** C++, x64 reverse engineering (IDA/Ghidra)

### What it does
- Reconstructs `MemoryPool::Alloc` from decompiled pseudo-C
- Documents 8-byte aligned allocation strategy
- Shows `VirtualAlloc`-based commit logic with overflow guard

### Why it's interesting
The original binary had no source or documentation — 
internal structure and behavior were recovered entirely 
from disassembly and decompiler output.

```c
void *__fastcall Mihoyo::Alloc(_QWORD *a1, __int64 a2, __int64 a3)
{
  size_t v4; // rsi
  _BYTE *v5; // r15
  _BYTE *v6; // rbx
  __int64 v7; // r14
  size_t v8; // rax
  _BYTE *v9; // rcx
  size_t v10; // rax
  SIZE_T v11; // r14
  SIZE_T v12; // r14

  v4 = (a3 * a2 + 7) & 0xFFFFFFFFFFFFFFF8uLL;
  v5 = (_BYTE *)a1[2];
  v6 = (_BYTE *)a1[3];
  v7 = a1[4];
  v8 = v5 - v6 - v7;
  v9 = v6;
  if ( v8 < v4 )
  {
    v10 = -v7 & (v4 + v7 + ~v8);
    v11 = 4 * v7;
    if ( v11 <= v10 )
      v11 = v10;
    if ( (unsigned __int64)&v5[v11] > *a1 + a1[5] )
      abort();
    if ( VirtualAlloc(v5, v11, 0x1000u, 4u) == v5 )
    {
      v12 = a1[2] + v11;
    }
    else
    {
      Log::Write(
        (__int64)"MemoryPool::InternalCommit failed: address 0x%08llx, size %zu, start 0x%08llx, current 0x%08llx, page size 0x%zx",
        *a1,
        v11,
        a1[2],
        a1[3],
        a1[4]);
      v12 = 0;
    }
    a1[2] = v12;
    v9 = (_BYTE *)a1[3];
  }
  a1[3] = &v9[v4];
  return memset(v6, 0, v4);
}
```
