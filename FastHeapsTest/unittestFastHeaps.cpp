#include "stdafx.h"
#include "CppUnitTest.h"
#include "ConcurrentFixedBlockHeap.h"
#include <thread>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace FastHeaps::ConcurrentFastHeap;

namespace FastHeapsTest
{
  void AllocDeallocIterationsFixedAllocator(TConcurrentFixedBlockHeap &heap, int iterations) {    
    void** ptrs = new void* [iterations];
    for (int i = 0; i < iterations; i++) {
      Pointer ptr = heap.Alloc();      
      ptrs[i] = ptr;
    }
    for (int i = 0; i < iterations; i++) {
      ConcurrentDeAlloc(ptrs[i]);
    }
    delete ptrs;
  }

  void AllocDeallocIterationsMalloc(int iterations) {
    void** ptrs = new void*[iterations];
    for (int i = 0; i < iterations; i++) {
      Pointer ptr = malloc(64);      
      ptrs[i] = ptr;
    }
    for (int i = 0; i < iterations; i++) {
      free(ptrs[i]);
    }
    delete ptrs;
  }

	TEST_CLASS(UnitTestFastHeaps)
	{
	public:
		
		TEST_METHOD(TestAllocDeAlloc)
		{
      TConcurrentFixedBlockHeap heap(1024, 1024);
      Pointer ptr = heap.Alloc();
      Assert::AreNotEqual((NativeUInt)nullptr, (NativeUInt)ptr);
      ConcurrentDeAlloc(ptr);
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
        ConcurrentDeAlloc(ptrs[i]);
      }
    }
    
    TEST_METHOD(TestFixedBlockConcurrentHeapPerformance)
    {      
      TConcurrentFixedBlockHeap heap(1024, 512);      
      AllocDeallocIterationsFixedAllocator(heap, 25000);
    }

    TEST_METHOD(TestMallocFreePerformance)
    {      
      const int iterations = 25000;
      AllocDeallocIterationsMalloc(iterations);      
    }

    TEST_METHOD(TestFixedBlockConcurrentHeapPerformanceThreaded)
    {
      const int iterations = 500000;
      TConcurrentFixedBlockHeap heap(64, 1024);
      std::thread* threads[10];
      for (int i = 0; i < 10; i++) {
        threads[i] = new std::thread(FastHeapsTest::AllocDeallocIterationsFixedAllocator, std::ref(heap), iterations);
      }  
      for (int i = 0; i < 10; i++) {        
        threads[i]->join();
      }
      for (int i = 0; i < 10; i++) {
        delete threads[i];
      }
      Assert::IsTrue(true);
    }

    TEST_METHOD(TestMallocPerformanceThreaded)
    {
      const int iterations = 500000;
      std::thread* threads[10];
      for (int i = 0; i < 10; i++) {
        threads[i] = new std::thread(FastHeapsTest::AllocDeallocIterationsMalloc, iterations);
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