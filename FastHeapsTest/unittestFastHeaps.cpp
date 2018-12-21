#include "stdafx.h"
#include "CppUnitTest.h"
#include "ConcurrentFastHeaps.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace FastHeapsTest
{		
	TEST_CLASS(UnitTestFastHeaps)
	{
	public:
		
		TEST_METHOD(TestAllocDeAlloc)
		{
      TConcurrentFixedBlockHeap heap(1024, 1024);
      Pointer ptr = heap.Alloc();
      Assert::AreNotEqual((NativeUInt)nullptr, (NativeUInt)ptr);
      heap.DeAlloc(ptr);
		}

    TEST_METHOD(TestIsLockFree)
    {
      TConcurrentFixedBlockHeap heap(1024, 1024);      
      Assert::IsTrue(heap.GetIsLockFree());      
    }
	};
}