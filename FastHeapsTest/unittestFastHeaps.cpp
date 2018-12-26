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

  void AllocDeallocIterationsUsingGlobalFixedAllocator(int iterations) {
    void** ptrs = new void*[10];
    for (int i = 0; i < iterations / 10; i++) {
      for (int j = 0; j < 10; j++) {
        ptrs[j] = Alloc(std::rand() % 4192);
      }
      for (int j = 0; j < 10; j++) {
        Free(ptrs[j]);
      }
    }
    delete ptrs;
  }

  void AllocDeallocIterationsUsingMalloc(int iterations) {
    void** ptrs = new void*[10];
    for (int i = 0; i < iterations / 10; i++) {
      for (int j = 0; j < 10; j++) {
        ptrs[j] = malloc(std::rand() % 4192);
      }
      for (int j = 0; j < 10; j++) {
        free(ptrs[j]);
      }
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

  TEST_MODULE_INITIALIZE(FastHeapsTestsInit) {
    FastHeaps::ConcurrentFixedBlockHeap::InitGlobalAllocators(1024);
  }

  TEST_MODULE_CLEANUP(FastHeapsTestsDone) {
    FastHeaps::ConcurrentFixedBlockHeap::DoneGlobalAllocators();
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

    TEST_METHOD(TestFixedBlockHeapPerformanceThreaded)
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

    TEST_METHOD(TestFixedBlockRandomGlobalHeapPerformanceThreaded)
    {
      const int iterations = 100000;
      std::thread* threads[10];
      for (int i = 0; i < 10; i++) {
        threads[i] = new std::thread(FastHeapsTest::AllocDeallocIterationsUsingGlobalFixedAllocator, iterations);
      }
      for (int i = 0; i < 10; i++) {
        threads[i]->join();
      }
      for (int i = 0; i < 10; i++) {
        delete threads[i];
      }
      Assert::IsTrue(true);
    }

    TEST_METHOD(TestMallocRandomPerformanceThreaded)
    {
      const int iterations = 100000;
      std::thread* threads[10];
      for (int i = 0; i < 10; i++) {
        threads[i] = new std::thread(FastHeapsTest::AllocDeallocIterationsUsingMalloc, iterations);
      }
      for (int i = 0; i < 10; i++) {
        threads[i]->join();
      }
      for (int i = 0; i < 10; i++) {
        delete threads[i];
      }
      Assert::IsTrue(true);
    }

    TEST_METHOD(TestMallocFixedSizePerformanceThreaded)
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

    TEST_METHOD(TestAllocGlobalHeaps)
    {
      for (int i = 1; i < 10000; i++) {
        Pointer ptr = FastHeaps::ConcurrentFixedBlockHeap::Alloc(i);
        Assert::AreNotEqual((NativeUInt)nullptr, (NativeUInt)ptr);
        Free(ptr);
      }
    }
	};
}