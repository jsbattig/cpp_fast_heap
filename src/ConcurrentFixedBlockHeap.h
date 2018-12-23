#pragma once

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

#include "PasTypes.h"
#include <limits.h>
#include "FastHeapTypes.h"
#include <atomic>

namespace FastHeaps {
  namespace ConcurrentFixedBlockHeap {

    struct Offset_t {
      long Serial;
      long Offset;
    };

    class TConcurrentFixedBlockHeap {
    protected:
      long FBlockCount;
      long FBlockSize;
      long FOriginalBlockSize;
      PPage CurrentPage;
      std::atomic<Offset_t> NextOffset;
      long FPageSize = 0;
      long FTotalUsableSize = 0;
      void AllocNewPage();
    public:
      TConcurrentFixedBlockHeap(long ABlockSize, long ABlockCount);
      ~TConcurrentFixedBlockHeap();
      Pointer Alloc();
      Boolean GetIsLockFree() { return NextOffset.is_lock_free(); }
      NativeUInt GetOriginalBlockSize() { return FOriginalBlockSize; }

      void* operator new(size_t size);
      void operator delete(void * p);
    };

    Boolean Free(Pointer Ptr);
    Pointer Alloc(long size);

    void InitGlobalAllocators(int BlocksPerHeap);
    void DoneGlobalAllocators();
  }
}