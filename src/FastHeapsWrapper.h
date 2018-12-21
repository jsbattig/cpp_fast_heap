#pragma once

#include "FastHeaps.h"

namespace HashTrie {
  public ref class TFixedBlockHeapWrapper
	{
  private:
    void* allocator;
	public:	    
    TFixedBlockHeapWrapper();
		~TFixedBlockHeapWrapper();
    intptr_t Alloc();
    void Dealloc(intptr_t ptr);
	};
}