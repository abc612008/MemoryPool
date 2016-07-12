#include <iostream>
#include <chrono>
#include <vector>
#include "../src/memorypool.h"
#include <cassert>
#include <crtdbg.h>
using namespace std;

void pause()
{
    system("pause");
}

void speedTest(MemoryPool& pool)
{
    const long testTime = 100000;
    const size_t testSize = 100000;
    {
        auto st = chrono::system_clock::now();
        for (long i = 0; i != testTime; i++)
        {
            char* c = new char[testSize];
            delete[] c;
        }
        auto ed = chrono::system_clock::now();
        cout << (ed - st).count() << endl;
    }

    {
        auto st = chrono::system_clock::now();
        for (long i = 0; i != testTime; i++)
        {
        }
        auto ed = chrono::system_clock::now();
        cout << (ed - st).count() << endl;
    }
}

int main()
{
    cout << "Functional test, press any key to start" << endl;

    MemoryPool pool(1024*1024); //1mb
    pause();
    {
        std::vector<int, MemoryPool::Allocator<int>> v(pool.getAllocator<int>());
        for (int i = 0; i != 10000; i++) v.push_back(i);
        for (int i = 0; i != 10000; i++, v.pop_back()) assert(v[v.size() - 1] == 9999 - i);
    }
    pause();

    //////////////////////////////////////
    cout << "Speed test" << endl;
    speedTest(pool);
    _CrtDumpMemoryLeaks();
    pause();
}
