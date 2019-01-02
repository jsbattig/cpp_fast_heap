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
#include <windows.h>
#include <memory>

namespace FastHeaps {
  namespace ConcurrentFixedBlockHeap {
    /* Globals */

    __inline Pointer AllocateMemory(NativeUInt ASize) {
      return malloc(ASize);
    }

    __inline void DeallocateMemory(Pointer APtr) {
      free(APtr);
    }

    static int AllocatorIndex[6144 + 1];
    static TConcurrentFixedBlockHeap* heaps[18];
    static Boolean inited = false;

    void InitGlobalAllocators(int BlocksPerHeap) {
      if (inited)
        return;
      for (int i = 0; i < sizeof(AllocatorIndex); i++) {
        if (i >= 0 && i <= 16) AllocatorIndex[i] = 0;
        else if (i > 17 && i <= 24) AllocatorIndex[i] = 1;
        else if (i > 24 && i <= 32) AllocatorIndex[i] = 2;
        else if (i > 32 && i <= 48) AllocatorIndex[i] = 3;
        else if (i > 48 && i <= 64) AllocatorIndex[i] = 4;
        else if (i > 64 && i <= 96) AllocatorIndex[i] = 5;
        else if (i > 96 && i <= 128) AllocatorIndex[i] = 6;
        else if (i > 128 && i <= 192) AllocatorIndex[i] = 7;
        else if (i > 192 && i <= 256) AllocatorIndex[i] = 8;
        else if (i > 256 && i <= 384) AllocatorIndex[i] = 9;
        else if (i > 384 && i <= 512) AllocatorIndex[i] = 10;
        else if (i > 512 && i <= 768) AllocatorIndex[i] = 11;
        else if (i > 768 && i <= 1024) AllocatorIndex[i] = 12;
        else if (i > 1024 && i <= 1536) AllocatorIndex[i] = 13;
        else if (i > 1536 && i <= 2048) AllocatorIndex[i] = 14;
        else if (i > 2048 && i <= 3072) AllocatorIndex[i] = 15;
        else if (i > 3072 && i <= 4096) AllocatorIndex[i] = 16;
        else if (i > 4096 && i <= 6144) AllocatorIndex[i] = 17;
      }
      for (int i = 0; i <= 8; i++) {
        heaps[i * 2] = new TConcurrentFixedBlockHeap(1 << (i + 3), BlocksPerHeap);
        heaps[i * 2 + 1] = new TConcurrentFixedBlockHeap((1 << (i + 3)) + ((1 << (i + 3)) / 2), BlocksPerHeap);
      }
      inited = true;
    }

    void DoneGlobalAllocators() {
      if (!inited)
        return;
      for (int i = 0; i <= 8; i++) {
        delete heaps[i * 2];
        delete heaps[i * 2 + 1];
      }
      inited = false;
    }

    Pointer Alloc(long size) {
      if(size < sizeof(AllocatorIndex) / sizeof(int))
        return heaps[AllocatorIndex[size]]->Alloc();
      else
      {
        PBlock Result = (PBlock)AllocateMemory(size + sizeof(TBlockHeader));
        Result->Header.PagePointer = nullptr;
        return &Result->Data;
      }
    }

    void Free(Pointer Ptr) {
      PBlock block = (PBlock)((NativeUInt)Ptr - sizeof(TBlockHeader));
      PPage page = block->Header.PagePointer;
      if (page != nullptr) {
        if (_InterlockedDecrement(&page->Header.RefCount) > 0)
          return;
        DeallocateMemory(page);
      } else
        DeallocateMemory(block);
    }

    /* TConcurrentFixedBlockHeap */

    void* TConcurrentFixedBlockHeap::operator new(size_t size) {
      return AllocateMemory(size);
    }

    void TConcurrentFixedBlockHeap::operator delete(void * p) {
      DeallocateMemory(p);
    }

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
      Offset_t nextOffset;
      PPage curPage;
      do {
        nextOffset = NextOffset;
        if (nextOffset.Offset >= FPageSize)
          continue;
        curPage = CurrentPage;
        Offset_t newNextOffset;
        newNextOffset.Serial = nextOffset.Serial + 1;
        newNextOffset.Offset = nextOffset.Offset + FBlockSize;
        NextOffset = newNextOffset;
        //if (_InterlockedCompareExchange64(&(*(volatile Int64*)(&NextOffset)), *(Int64*)(&newNextOffset), *(Int64*)(&nextOffset)) == *(Int64*)(&nextOffset)) {
          if (newNextOffset.Offset >= FPageSize)
            AllocNewPage();
          break;
        //}
      } while (true);
      PBlock Result = (PBlock)((NativeUInt)curPage + nextOffset.Offset);
      Result->Header.PagePointer = curPage;
      return &Result->Data;
    }
  }
}