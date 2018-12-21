#pragma once

#include "PasTypes.h"
#include <limits.h>
#include "FastHeaps.h"
#include <mutex>
#include <atomic>

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

struct TPageMetadata {  
  TPage* CurrentPagePtr;
  NativeUInt NextOffset;   
};

class TConcurrentFastHeap {
protected:
  std::atomic<int> AllocEntryCount;
  std::mutex FNextBlockMutex;
  std::atomic<TPageMetadata> CurrentPage;  
  NativeUInt FPageSize = 0;
  NativeUInt FTotalUsableSize = 0;
  void AllocateMemory(void* &APtr, NativeUInt ASize);
  void TryAllocNewBlockArray();
  void DeallocateMemory(void* APtr);
public:
  TConcurrentFastHeap();
  virtual ~TConcurrentFastHeap();
  void DeAlloc(Pointer Ptr);
  Integer GetCurrentBlockRefCount();
  Boolean GetIsLockFree() { return CurrentPage.is_lock_free(); }
};

class TConcurrentFixedBlockHeap : public TConcurrentFastHeap {
protected:
  NativeUInt FBlockSize;
  NativeUInt FOriginalBlockSize;
public:
  TConcurrentFixedBlockHeap(NativeUInt ABlockSize, NativeUInt ABlockCount);
  Pointer Alloc();
  NativeUInt GetOriginalBlockSize() { return FOriginalBlockSize; }
};

class TConcurrentVariableBlockHeap : TConcurrentFastHeap {
public:
  TConcurrentVariableBlockHeap(NativeUInt APoolSize);
  Pointer Alloc(NativeUInt ASize);
};

Boolean ConcurrentDeAlloc(Pointer Ptr);