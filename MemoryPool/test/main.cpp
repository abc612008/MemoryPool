#include <iostream>
#include <chrono>
#include "../src/memorypool.h"
#include <cassert>
#include <crtdbg.h>

constexpr size_t poolSize = 1024;
using FixedPool = FixedMemoryPool<int, poolSize>;
using namespace std;

void pause()
{
    system("pause");
}

//Frame
void speedTest()
{
    const long testTime = 100000000;
    {
        auto st = chrono::system_clock::now();
        for (long i = 0; i != testTime; i++)
        {
        }
        auto ed = chrono::system_clock::now();
        cout << (ed - st).count() << endl;
    }
}

struct BigPacket
{
    int arr[1024];
};

int main()
{
    cout << "Functional test, press any key to start" << endl;
    pause();
    {
        cout << "Test 1. Memory Pool" << endl;
        MemoryPool pool(8 * 1024);
        cout << "Checkpoint 1 ";
        {
            auto alloc = pool.getAllocator<int>();
            int* ptr = alloc.newobj();
            *ptr = 1;
            (*ptr)++;
            assert(*ptr == 2);
            alloc.delobj(ptr);
        }
        cout << "OK..." << endl << "Checkpoint 2 ";
        {
            auto alloc = pool.getAllocator<int>();
            auto alloc2 = pool.getAllocator<BigPacket>();
            auto ptr1 = alloc.newobj();
            auto ptr2 = alloc2.newobj();
            ptr2->arr[0] = 1;
            ptr2->arr[1] = 2;
            *ptr1= ptr2->arr[0] + ptr2->arr[1];
            assert(*ptr1 == 3);
            alloc.delobj(ptr1);
            alloc2.delobj(ptr2);
        }
        cout << "OK..." << endl << "Checkpoint 3 ";
        {
            auto alloc = pool.getAllocator<int>();
            auto alloc2 = pool.getAllocator<BigPacket>();
            auto alloc3 = pool.getAllocator<BigPacket>();
            auto ptr1 = alloc.newobj();
            auto ptr2 = alloc2.newobj();
            auto ptr3 = alloc3.newobj();
            ptr2->arr[0] = 1;
            ptr3->arr[1] = 2;
            *ptr1 = ptr2->arr[0] + ptr3->arr[1];
            assert(*ptr1 == 3);
            alloc.delobj(ptr1);
            alloc2.delobj(ptr2);
            alloc3.delobj(ptr3);
        }
        cout << "OK..." << endl << "Checkpoint 4 " ;
        {
            auto alloc = pool.getAllocator<int>();
            auto alloc2 = pool.getAllocator<BigPacket>();
            auto ptr1 = alloc.newobj();
            auto ptr2 = alloc2.newobj();
            alloc.delobj(ptr3);
            alloc2.delobj(ptr2);
            auto ptr3 = alloc.newobj();
            assert(ptr3 == ptr1);
            alloc.delobj(ptr1);
        }
        cout << "OK..." << endl << "Checkpoint 5 ";
        {
            auto alloc = pool.getAllocator<int>();
            auto alloc2 = pool.getAllocator<BigPacket>();
            auto ptr1 = alloc.newobj();
            auto ptr2 = alloc2.newobj();
            auto ptr3 = alloc2.newobj();
            ptr2->arr[0] = 1;
            ptr3->arr[1] = 2;
            *ptr1 = ptr2->arr[0] + ptr3->arr[1];
            assert(*ptr1 == 3);
            alloc.delobj(ptr1);
            alloc2.delobj(ptr2);
            alloc2.delobj(ptr3);
        }
        cout << "OK..." << endl << "Checkpoint 6 ";
        {
            constexpr int testnum = 10000;
            auto alloc = pool.getAllocator<int>();
            auto alloc2 = pool.getAllocator<BigPacket>();
            void* ptrs[testnum] = {};
            for (auto i = 0; i != testnum; i++)
            {
                ptrs[i] = i % 2 == 0 ? static_cast<void*>(alloc.newobj()) : static_cast<void*>(alloc2.newobj());
            }
            for (auto i = 0; i != testnum; i++)
            {
                i % 2 == 0 ? alloc.delobj(static_cast<int*>(ptrs[i])) : alloc2.delobj(static_cast<BigPacket*>(ptrs[i]));
            }
        }
        cout << "OK..." << endl;
    }
    {
        cout << "Test 2. Fixed Memory Pool" << endl;
        cout << "Checkpoint 1 ";
        {
            auto alloc = FixedAllocator<int, 16>();
            int* ptr = alloc.newobj();
            *ptr = 1;
            (*ptr)++;
            assert(*ptr == 2);
            alloc.delobj(ptr);
        }
        cout << "OK..." << endl << "Checkpoint 2 ";
        {
            auto alloc = FixedAllocator<int, 16>();
            auto ptr1 = alloc.newobj();
            auto ptr2 = alloc.newobj();
            *ptr2 = 1;
            *ptr1 = (*ptr2)++;
            assert(*ptr1+*ptr2 == 3);
            alloc.delobj(ptr1);
            alloc.delobj(ptr2);
        }
        cout << "OK..." << endl << "Checkpoint 3 ";
        {
            auto alloc = FixedAllocator<int, 2>();
            auto ptr1 = alloc.newobj();
            auto ptr2 = alloc.newobj();
            auto ptr3 = alloc.newobj();
            *ptr1 = 1;
            *ptr2 = 2;
            *ptr3 = *ptr1 + *ptr2;
            assert(*ptr3 == 3);
            alloc.delobj(ptr1);
            alloc.delobj(ptr2);
            alloc.delobj(ptr3);
        }
        cout << "OK..." << endl << "Checkpoint 4 ";
        {
            auto alloc = FixedAllocator<int, 2>();
            auto ptr1 = alloc.newobj();
            alloc.delobj(ptr1);
            auto ptr2 = alloc.newobj();
            assert(ptr2 == ptr1);
            alloc.delobj(ptr2);
        }
        cout << "OK..." << endl << "Checkpoint 5 ";
        {
            constexpr int testnum = 10000;
            auto alloc = FixedAllocator<int, 2500>();
            auto alloc2 = FixedAllocator<BigPacket, 2500>();
            void* ptrs[testnum] = {};
            for (auto i = 0; i != testnum; i++)
            {
                ptrs[i] = i % 2 == 0 ? static_cast<void*>(alloc.newobj()) : static_cast<void*>(alloc2.newobj());
            }
            for (auto i = 0; i != testnum; i++)
            {
                i % 2 == 0 ? alloc.delobj(static_cast<int*>(ptrs[i])) : alloc2.delobj(static_cast<BigPacket*>(ptrs[i]));
            }
        }
        cout << "OK..." << endl;
    }
    pause();

    //////////////////////////////////////
    cout << "Speed test" << endl;
    speedTest();
    _CrtDumpMemoryLeaks();
    pause();
}
