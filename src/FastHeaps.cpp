#include "stdafx.h"

#include "FastHeaps.h"
#include <stdlib.h>

Boolean DeAlloc(Pointer Ptr) {
  Ptr = *(PPointer)((NativeUInt)Ptr - sizeof(TBlockHeader));
  (*((PNativeUInt)Ptr))--;
  if (*(PNativeUInt)Ptr > 0)
    return false;
  free(Ptr);
  return true;
}

/* TFastHeap */

Pointer AllocBlockInPage(Pointer APage, NativeUInt AOffset) {  
  Pointer Result = (Pointer)((NativeUInt)APage + AOffset);
  ((PBlock)Result)->Header.PagePointer = (PPage)APage; 
  ((PPage)APage)->Header.RefCount++;
  return &((PBlock)Result)->Data;
}

TFastHeap::~TFastHeap() {  
  if (FStartBlockArray != nullptr) {   
    ((PPage)FStartBlockArray)->Header.RefCount--;
    if (((PPage)FStartBlockArray)->Header.RefCount <= 0)
      DeallocateMemory(FStartBlockArray);
  }  
}

void TFastHeap::AllocNewBlockArray() {  
  if (FStartBlockArray != nullptr && ((PPage)FStartBlockArray)->Header.RefCount == 1)
    FNextOffset = sizeof(TPageHeader);
  else {    
    if (FStartBlockArray != nullptr)
      ((PPage)FStartBlockArray)->Header.RefCount--;
    AllocateMemory(FStartBlockArray, FPageSize);
    ((PPage)FStartBlockArray)->Header.RefCount = 1;
    FNextOffset = sizeof(TPageHeader);
  }
}

void TFastHeap::DeAlloc(Pointer Ptr) { 
  ::DeAlloc(Ptr);   
}

void TFastHeap::AllocateMemory(Pointer &APtr, NativeUInt ASize) { 
  APtr = malloc(ASize);  
}

void TFastHeap::DeallocateMemory(Pointer APtr) {  
  free(APtr);
}

Integer TFastHeap::GetCurrentBlockRefCount() {
  return ((PPage)FStartBlockArray)->Header.RefCount;
}

/* TFixedBlockHeap */

TFixedBlockHeap::TFixedBlockHeap(NativeUInt ABlockSize, NativeUInt ABlockCount) {
  FOriginalBlockSize = ABlockSize;
  FBlockSize = (ABlockSize + sizeof(TBlockHeader) + Aligner) & (~Aligner);
  FTotalUsableSize = FBlockSize * ABlockCount;
  FPageSize = FTotalUsableSize + sizeof(TPageHeader);
  FNextOffset = FPageSize;
}

Pointer TFixedBlockHeap::Alloc() {
  if (FNextOffset >= FPageSize)
    AllocNewBlockArray();
  Pointer Result = AllocBlockInPage(FStartBlockArray, FNextOffset);
  FNextOffset += FBlockSize;
  return Result;
}

/* TVariableBlockHeap */

TVariableBlockHeap::TVariableBlockHeap(NativeUInt APoolSize) { 
  FTotalUsableSize = (APoolSize + Aligner) & (~Aligner);
  FPageSize = FTotalUsableSize + sizeof(TPageHeader);
  FNextOffset = FPageSize;
}

Pointer TVariableBlockHeap::Alloc(NativeUInt ASize) {  
  Pointer Result;
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
  return Result;
}