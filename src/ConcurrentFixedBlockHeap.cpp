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
      TBlock* block = (TBlock*)((NativeUInt)Ptr - sizeof(TBlockHeader));
      TPageHeader* page = (TPageHeader*)block->Header.PagePointer;
      if (_InterlockedDecrement(&page->RefCount) > 0)
        return false;
      DeallocateMemory(page);
      return true;
    }

    Pointer ConcurentAllocBlockInPage(PPage APage, NativeUInt AOffset) {
      PBlock Result = (PBlock)((NativeUInt)APage + AOffset);
      Result->Header.PagePointer = APage;
      return &Result->Data;
    }

    /* TConcurrentFixedBlockHeap */

    TConcurrentFixedBlockHeap::TConcurrentFixedBlockHeap(NativeUInt ABlockSize, long ABlockCount) {
      FOriginalBlockSize = ABlockSize;
      FBlockCount = ABlockCount;
      FBlockSize = (ABlockSize + sizeof(TBlockHeader) + Aligner) & (~Aligner);
      FTotalUsableSize = FBlockSize * ABlockCount;
      FPageSize = FTotalUsableSize + sizeof(TPageHeader);
      TPageMetadata page = { nullptr, FPageSize };
      CurrentPage = page;
    }

    TConcurrentFixedBlockHeap::~TConcurrentFixedBlockHeap() {
      TPageMetadata page = CurrentPage;
      if (page.CurrentPagePtr != nullptr && _InterlockedDecrement(&page.CurrentPagePtr->Header.RefCount) <= 0)
        DeallocateMemory(page.CurrentPagePtr);
    }

    void TConcurrentFixedBlockHeap::TryAllocNewBlockArray() {
      TPageMetadata page = CurrentPage;
      if (page.CurrentPagePtr != nullptr && page.CurrentPagePtr->Header.RefCount == 1) {
        /* This happens when we happen to be at the end of the page but the Heap itself is the
           only object referencing the page. In that case we can simply reset the offset variable and RefCount */
        TPageMetadata newPage = { page.CurrentPagePtr, sizeof(TPageHeader) };
        page.CurrentPagePtr->Header.RefCount = FBlockCount + 1;
        if (!CurrentPage.compare_exchange_weak(page, newPage))
          DeallocateMemory(page.CurrentPagePtr);
      }  else {
        TPageMetadata newPage = { (PPage)AllocateMemory(FPageSize), sizeof(TPageHeader) };
        newPage.CurrentPagePtr->Header.RefCount = FBlockCount + 1;
        if (!CurrentPage.compare_exchange_weak(page, newPage)) {
          DeallocateMemory(newPage.CurrentPagePtr);
          return;
        }
        if (page.CurrentPagePtr != nullptr && _InterlockedDecrement(&page.CurrentPagePtr->Header.RefCount) == 0)
          DeallocateMemory(page.CurrentPagePtr);
      }
    }

    Integer TConcurrentFixedBlockHeap::GetCurrentBlockRefCount() {
      TPageMetadata page = CurrentPage;
      return page.CurrentPagePtr->Header.RefCount;
    }

    Pointer TConcurrentFixedBlockHeap::Alloc() {
      TPageMetadata page;
      do {
        page = CurrentPage;
        if (page.NextOffset >= FPageSize) {
          TryAllocNewBlockArray();
          continue;
        }
        TPageMetadata newPage = { page.CurrentPagePtr, page.NextOffset + FBlockSize };
        if (CurrentPage.compare_exchange_weak(page, newPage))
          break;
      } while (true);
      return ConcurentAllocBlockInPage(page.CurrentPagePtr, page.NextOffset);
    }
  }
}