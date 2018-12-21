#pragma once

#include "PasTypes.h"
#include <limits.h>

/*
  The MIT License(MIT)

  Copyright(c) 2015 Jose Sebastian Battig

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files(the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

const int _16KB = 16 * 1024;
const int _32KB = 32 * 1024;
const int _64KB = 64 * 1024;
const int Aligner = sizeof(NativeUInt) - 1;

typedef struct TPage* PPage;
typedef struct TBlock* PBlock;

struct TBlockHeader {
  PPage PagePointer;
};

struct TBlock {
  TBlockHeader Header;  
  Byte Data[INT_MAX - sizeof(TBlockHeader)];
};

struct TPageHeader {
  Pointer Allocator;
  volatile long RefCount;
};

struct TPage {
  TPageHeader Header;
  TBlock FirstBlock;
};

class TFastHeap {
protected:
  Pointer FStartBlockArray;
  NativeUInt FNextOffset;
  NativeUInt FPageSize;
  NativeUInt FTotalUsableSize;
  void AllocateMemory(void* &APtr, NativeUInt ASize);
  void AllocNewBlockArray();
  void DeallocateMemory(void* APtr);
public:
  virtual ~TFastHeap();
  void DeAlloc(Pointer Ptr);
  Integer GetCurrentBlockRefCount();
};

class TFixedBlockHeap : TFastHeap {
protected:
  NativeUInt FBlockSize;
  NativeUInt FOriginalBlockSize;
public:
  TFixedBlockHeap(NativeUInt ABlockSize, NativeUInt ABlockCount);  
  Pointer Alloc();
  NativeUInt GetOriginalBlockSize() { return FOriginalBlockSize; }
};

class TVariableBlockHeap : TFastHeap {
public:
  TVariableBlockHeap(NativeUInt APoolSize);
  Pointer Alloc(NativeUInt ASize);
};

Boolean DeAlloc(Pointer Ptr);
Pointer AllocBlockInPage(Pointer APage, NativeUInt AOffset);