#include "stdafx.h"

#include "FastHeapsWrapper.h"
#include "HashTrie.h"
#include "TrieAllocators.h"
#include <malloc.h>

namespace HashTrie {
  TFixedBlockHeapWrapper::TFixedBlockHeapWrapper()
	{
    allocator = new TFixedBlockHeap(10, 10);
	}


  TFixedBlockHeapWrapper::~TFixedBlockHeapWrapper()
	{
    delete allocator;
	}

  intptr_t TFixedBlockHeapWrapper::Alloc()
	{
    return (Int64)((TFixedBlockHeap*)allocator)->Alloc();
	}

  void TFixedBlockHeapWrapper::Dealloc(intptr_t ptr) {
    ::DeAlloc((void*)ptr);
  }
}