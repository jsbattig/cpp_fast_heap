#include "stdafx.h"

#include "ConcurrentFastHeaps.h"
#include <stdlib.h>

Boolean ConcurrentDeAlloc(Pointer Ptr) {
  Ptr = *(PPointer)((NativeUInt)Ptr - sizeof(TBlockHeader));
  if (_InterlockedDecrement(&(*((volatile long*)Ptr))) > 0)  
    return false;
  free(Ptr);
  return true;
}

Pointer ConcurentAllocBlockInPage(Pointer APage, NativeUInt AOffset) {
  Pointer Result = (Pointer)((NativeUInt)APage + AOffset);
  ((PBlock)Result)->Header.PagePointer = (PPage)APage;
  _InterlockedIncrement(&((PPage)APage)->Header.RefCount); 
  return &((PBlock)Result)->Data;
}

/* TConcurrentFastHeap */

TConcurrentFastHeap::TConcurrentFastHeap() {
  TPageMetadata page = { nullptr, 0 };
  CurrentPage = page;
  AllocEntryCount = 0;
}

TConcurrentFastHeap::~TConcurrentFastHeap() {
  TPageMetadata page = CurrentPage;
  if (page.CurrentPagePtr != nullptr) {
    page.CurrentPagePtr->Header.RefCount--;
    if (page.CurrentPagePtr->Header.RefCount <= 0)
      DeallocateMemory(page.CurrentPagePtr);
  }  
}

void TConcurrentFastHeap::TryAllocNewBlockArray() {
  TPageMetadata page = CurrentPage;
  if (page.CurrentPagePtr != nullptr && page.CurrentPagePtr->Header.RefCount == 1) {
    /* This happens when we happen to be at the end of the page but the Heap itself is the
       only object referencing the page. In that case we can simply reset the offset variable */
    TPageMetadata newPage = page;
    newPage.NextOffset = sizeof(TPageHeader);
    CurrentPage.compare_exchange_strong(page, newPage);    
  }
  else {    
    TPageMetadata newPage = page;
    if (newPage.CurrentPagePtr != nullptr)
      _InterlockedDecrement(&newPage.CurrentPagePtr->Header.RefCount);
    Pointer ptr;
    AllocateMemory(ptr, FPageSize);
    newPage.CurrentPagePtr = (PPage)ptr;
    newPage.CurrentPagePtr->Header.RefCount = 1;
    newPage.NextOffset = sizeof(TPageHeader);
    if (!CurrentPage.compare_exchange_strong(page, newPage))
      free(ptr);
  }
}

void TConcurrentFastHeap::DeAlloc(Pointer Ptr) {
  ::ConcurrentDeAlloc(Ptr);   
}

void TConcurrentFastHeap::AllocateMemory(Pointer &APtr, NativeUInt ASize) {
  APtr = malloc(ASize);  
}

void TConcurrentFastHeap::DeallocateMemory(Pointer APtr) {
  free(APtr);
}

Integer TConcurrentFastHeap::GetCurrentBlockRefCount() {
  TPageMetadata page = CurrentPage;
  return page.CurrentPagePtr->Header.RefCount;
}

/* TConcurrentFixedBlockHeap */

TConcurrentFixedBlockHeap::TConcurrentFixedBlockHeap(NativeUInt ABlockSize, NativeUInt ABlockCount) {  
  FOriginalBlockSize = ABlockSize;
  FBlockSize = (ABlockSize + sizeof(TBlockHeader) + Aligner) & (~Aligner);
  FTotalUsableSize = FBlockSize * ABlockCount;
  FPageSize = FTotalUsableSize + sizeof(TPageHeader);
  TPageMetadata page = { nullptr, FPageSize };
  CurrentPage = page;
}

Pointer TConcurrentFixedBlockHeap::Alloc() {  
  if (AllocEntryCount.fetch_add(1) == 0)
    /* Let's free here any pointers in purgatory */;    
  TPageMetadata page;
  do {    
    page = CurrentPage;
    if (page.NextOffset >= FPageSize) {
      TryAllocNewBlockArray();
      continue;
    }
    TPageMetadata newPage = { page.CurrentPagePtr, page.NextOffset + FBlockSize };
    if (CurrentPage.compare_exchange_strong(page, newPage))
      break;
  } while (true);
  Pointer Result = ConcurentAllocBlockInPage(page.CurrentPagePtr, page.NextOffset);
  AllocEntryCount--;
  return Result;
}

/* TConcurrentVariableBlockHeap */

TConcurrentVariableBlockHeap::TConcurrentVariableBlockHeap(NativeUInt APoolSize) {
  FTotalUsableSize = (APoolSize + Aligner) & (~Aligner);
  FPageSize = FTotalUsableSize + sizeof(TPageHeader);
  TPageMetadata page = CurrentPage;
  page.NextOffset = FPageSize;
  CurrentPage = page;
}

Pointer TConcurrentVariableBlockHeap::Alloc(NativeUInt ASize) {  
  /*Pointer Result;
  ASize = (ASize + sizeof(TBlockHeader) + Aligner) & (~Aligner); // Align size to native word size bits
  if (ASize <= FTotalUsableSize) {
    if (FNextOffset + ASize >= FPageSize)
      AllocNewBlockArray();
    Result = AllocBlockInPage(FStartBlockArray, FNextOffset);
    FNextOffset += ASize;
  }
  else {
    AllocateMemory(Result, sizeof(TPageHeader) + ASize);
    ((PPage)Result)->Header.RefCount = 0;
    Result = AllocBlockInPage(PPage(Result), sizeof(TPageHeader));
  }  
  return Result;*/
  return nullptr;
}