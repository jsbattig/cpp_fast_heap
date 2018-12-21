#include "stdafx.h"
#include "CppUnitTest.h"
#include "ConcurrentFastHeaps.h"
#include <thread>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace FastHeapsTest
{
  void threaded_method(TConcurrentFixedBlockHeap &heap) {
    const int iterations = 25000;
    void* ptrs[iterations];
    for (int i = 0; i < iterations; i++) {
      Pointer ptr = heap.Alloc();
      Assert::AreNotEqual((NativeUInt)nullptr, (NativeUInt)ptr);
      ptrs[i] = ptr;
    }
    for (int i = 0; i < iterations; i++) {
      heap.DeAlloc(ptrs[i]);
    }
  }

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

    TEST_METHOD(TestAllocDeAllocForcingFreePage)
    {
      void* ptrs[15];
      TConcurrentFixedBlockHeap heap(1024, 10);
      for (int i = 0; i < 15; i++) {
        Pointer ptr = heap.Alloc();
        Assert::AreNotEqual((NativeUInt)nullptr, (NativeUInt)ptr);        
        ptrs[i] = ptr;
      }
      for (int i = 0; i < 15; i++) {
        heap.DeAlloc(ptrs[i]);
      }
    }
    
    TEST_METHOD(TestFixedBlockConcurrentHeapPerformance)
    {      
      TConcurrentFixedBlockHeap heap(1024, 512);      
      threaded_method(heap);
    }

    TEST_METHOD(TestMallocFreePerformance)
    {
      const int iterations = 250000;
      void* ptrs[iterations];      
      for (int i = 0; i < iterations; i++) {
        Pointer ptr = malloc(1024);
        Assert::AreNotEqual((NativeUInt)nullptr, (NativeUInt)ptr);
        ptrs[i] = ptr;
      }
      for (int i = 0; i < iterations; i++) {
        free(ptrs[i]);
      }
    }
    TEST_METHOD(TestFixedBlockConcurrentHeapPerformanceThreaded)
    {
      const int iterations = 25000;
      void* ptrs[iterations];
      TConcurrentFixedBlockHeap heap(1024, 512);
      std::thread* threads[10];
      for (int i = 0; i < 10; i++) {
        threads[i] = new std::thread(FastHeapsTest::threaded_method, std::ref(heap));        
      }  
      for (int i = 0; i < 10; i++) {        
        threads[i]->join();
      }
      for (int i = 0; i < 10; i++) {
        delete threads[i];
      }
      Assert::IsTrue(true);
    }    
	};
}