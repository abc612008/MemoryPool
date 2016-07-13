#include <iostream>
#include <chrono>
#include "../src/memorypool.h"
#include <cassert>
#include <crtdbg.h>

constexpr size_t poolSize = 1024;
using FixedPool = FixedMemoryPool<int, poolSize>;

namespace mp2
{
#include "mp2.h"
}
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
        cout << "Checkpoint 1" << endl;
        {
            auto alloc = pool.getAllocator<int>();
            int* ptr = alloc.newobj();
            *ptr = 1;
            (*ptr)++;
            assert(*ptr == 2);
            alloc.delobj(ptr);
        }
        cout << "Checkpoint 2" << endl;
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
        cout << "Checkpoint 3" << endl;
        {
            auto alloc = pool.getAllocator<int>();
            auto alloc2 = pool.getAllocator<BigPacket>();
            auto alloc3 = pool.getAllocator<BigPacket>();
            auto ptr1 = alloc.newobj();
            auto ptr2 = alloc2.newobj(), ptr3 = alloc3.newobj();
            ptr2->arr[0] = 1;
            ptr3->arr[1] = 2;
            *ptr1 = ptr2->arr[0] + ptr3->arr[1];
            assert(*ptr1 == 3);
            alloc.delobj(ptr1);
            alloc2.delobj(ptr2);
            alloc3.delobj(ptr3);
        }
        cout << "Checkpoint 4" << endl;
        {
            auto alloc = pool.getAllocator<int>();
            auto alloc2 = pool.getAllocator<BigPacket>();
            auto alloc3 = pool.getAllocator<BigPacket>();
            auto ptr1 = alloc.newobj();
            auto ptr2 = alloc2.newobj();
            alloc.delobj(ptr1);
            alloc2.delobj(ptr2);
            auto ptr3 = alloc.newobj();
            assert(ptr3 == ptr1);
            alloc.delobj(ptr1);
        }
        cout << "Checkpoint 5" << endl;
    }
    pause();

    //////////////////////////////////////
    cout << "Speed test" << endl;
    speedTest();
    _CrtDumpMemoryLeaks();
    pause();
}
