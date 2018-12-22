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

#include "stdafx.h"
#include "ConcurrentFixedBlockHeap.h"
#include <stdlib.h>

namespace FastHeaps {
  namespace ConcurrentFastHeap {
    /* Globals */

    __inline Pointer AllocateMemory(NativeUInt ASize) {
      return malloc(ASize);
    }

    __inline void DeallocateMemory(Pointer APtr) {
      free(APtr);
    }

    Boolean ConcurrentDeAlloc(Pointer Ptr) {
      PBlock block = (PBlock)((NativeUInt)Ptr - sizeof(TBlockHeader));
      PPage page = block->Header.PagePointer;
      if (_InterlockedDecrement(&page->Header.RefCount) > 0)
        return false;
      DeallocateMemory(page);
      return true;
    }

    /* TConcurrentFixedBlockHeap */

    TConcurrentFixedBlockHeap::TConcurrentFixedBlockHeap(long ABlockSize, long ABlockCount) {
      FOriginalBlockSize = ABlockSize;
      FBlockCount = ABlockCount;
      FBlockSize = (ABlockSize + sizeof(TBlockHeader) + Aligner) & (~Aligner);
      FTotalUsableSize = FBlockSize * ABlockCount;
      FPageSize = FTotalUsableSize + sizeof(TPageHeader);
      AllocNewPage();
    }

    TConcurrentFixedBlockHeap::~TConcurrentFixedBlockHeap() {
      DeallocateMemory(CurrentPage);
    }

    void TConcurrentFixedBlockHeap::AllocNewPage() {
      PPage newPage = (PPage)AllocateMemory(FPageSize);
      newPage->Header.RefCount = FBlockCount;
      CurrentPage = newPage;
      Offset_t offset = NextOffset;
      offset.Serial++;
      offset.Offset = sizeof(TPageHeader);
      NextOffset = offset;
    }

    Pointer TConcurrentFixedBlockHeap::Alloc() {
      long pageSize = FPageSize;
      long blockSize = FBlockSize;
      Offset_t nextOffset;
      long nextOffsetOffset;
      PPage curPage;
      do {
        nextOffset = NextOffset;
        nextOffsetOffset = nextOffset.Offset;
        if (nextOffsetOffset >= pageSize)
          continue;
        curPage = CurrentPage;
        Offset_t newNextOffset;
        newNextOffset.Serial = nextOffset.Serial + 1;
        long newNextOffsetOffset = nextOffsetOffset + blockSize;
        newNextOffset.Offset = newNextOffsetOffset;
        if (NextOffset.compare_exchange_weak(nextOffset, newNextOffset)) {
          if (newNextOffsetOffset >= pageSize)
            AllocNewPage();
          break;
        }
      } while (true);
      PBlock Result = (PBlock)((NativeUInt)curPage + nextOffsetOffset);
      Result->Header.PagePointer = curPage;
      return &Result->Data;
    }
  }
}