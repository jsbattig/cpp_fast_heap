#include "stdafx.h"
#include "CppUnitTest.h"
#include "ConcurrentFixedBlockHeap.h"
#include <thread>
#include <atomic>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace FastHeaps::ConcurrentFixedBlockHeap;

namespace FastHeapsTest
{
  void AllocDeallocIterationsFixedAllocator(TConcurrentFixedBlockHeap &heap, int iterations) {    
    void** ptrs = new void* [iterations];
    for (int i = 0; i < iterations; i++) {
      ptrs[i] = heap.Alloc();
    }
    for (int i = 0; i < iterations; i++) {
      Free(ptrs[i]);
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
      Free(ptr);
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
        Free(ptrs[i]);
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
      const int iterations = 50000;
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
      const int iterations = 50000;
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
    
    TEST_METHOD(TestInitAtomicIntInPlace)
    {
      struct T {
        std::atomic<int> i;
      };
      void* p = malloc(sizeof(std::atomic<int>));
      Assert::AreNotEqual(0, (int)(*(std::atomic<int>*)p));
      (*(std::atomic<int>*)p) = 0;
      Assert::AreEqual(sizeof(int), sizeof(std::atomic<int>));
      Assert::AreEqual(0, (int)(*(std::atomic<int>*)p));
    }

    TEST_METHOD(TestAllocGlobalHeaps)
    {
      FastHeaps::ConcurrentFixedBlockHeap::InitGlobalAllocators(256);
      for (int i = 1; i < 10000; i++) {
        Pointer ptr = FastHeaps::ConcurrentFixedBlockHeap::Alloc(i);
        Assert::AreNotEqual((NativeUInt)nullptr, (NativeUInt)ptr);
        Free(ptr);
      }
      FastHeaps::ConcurrentFixedBlockHeap::DoneGlobalAllocators();
    }
	};
}