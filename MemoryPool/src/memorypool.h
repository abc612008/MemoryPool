#ifndef MEMORYPOOL_H__
#define MEMORYPOOL_H__
#include "Allocators.h"
#include <stdlib.h>
#include <utility>

class MemoryPool
{
public:

    explicit MemoryPool(int unitPoolSize) :
        m_buffer(new char[unitPoolSize]),
        m_bufferSize(unitPoolSize),
        m_allocateMap(static_cast<AllocInfo*>(malloc(32 * sizeof(AllocInfo)))),
        m_allocateMapSizeAlloc(32)
    {
    }
    MemoryPool(const MemoryPool&) = delete;
    MemoryPool(MemoryPool&&) = delete;
    MemoryPool& operator = (const MemoryPool&) = delete;
    ~MemoryPool()
    {
        free(m_allocateMap);
        delete[] m_buffer;
        delete m_next;
    }

    // Get an allocator
    template <class T>
    Allocator<T> getAllocator()
    {
        return Allocator<T>(this);
    }

    // Allocate memory
    template <class T>
    T* allocate(std::size_t size)
    {
        if (size == 0) return nullptr;

        bool notEnough = false; // If need create a new memory pool
        char* pos = m_buffer;
        if(size>m_maxContinuousMemorySize&&m_maxContinuousMemorySizeValid)
        {
            notEnough = true;
        }
        else
        {
            if (m_allocateMapSize > 0)
            {
                if (m_allocateMapSize > 1)
                {
                    const AllocInfo* lastInfo = m_allocateMap;
                    auto ptr = m_allocateMap + 1;
                    for (; ptr != m_allocateMap + m_allocateMapSize; ++ptr)
                    {
                        if (lastInfo->first + lastInfo->second + size < ptr->first)
                        {
                            pos = lastInfo->first + lastInfo->second;
                            break;
                        }
                        lastInfo = ptr;
                    }
                    if (ptr == m_allocateMap + m_allocateMapSize)
                    {
                        if (lastInfo->first + lastInfo->second + size < m_buffer + m_bufferSize)
                        {
                            pos = lastInfo->first + lastInfo->second;
                        }
                        else
                        {
                            notEnough = true;
                            m_maxContinuousMemorySize = size;
                        }
                    }
                }
                else
                {
                    pos += m_allocateMap->second;
                }
            }
        }
        if(notEnough)
        {
            if (!m_next)
            {
                size_t newSize = m_bufferSize * 2;
                while (size > m_bufferSize) newSize *= 2;
                m_next = new MemoryPool(newSize);
            }
            m_maxContinuousMemorySize = size > m_maxContinuousMemorySize ? m_maxContinuousMemorySize : size - 1;
            return m_next->allocate<T>(size);
        }
        insertAllocateMap(pos, size);
        return reinterpret_cast<T*>(pos);
    }

    // Deallocate memory
    template <class T>
    void deallocate(T* p, std::size_t size)
    {
        m_maxContinuousMemorySizeValid = false; // Make it invalid
        int index = binarySearch(reinterpret_cast<char*>(p));
        if (index == -1|| m_allocateMap[index].second!= size)
        {
            if (m_next)
            {
                m_next->deallocate(p, size);
                return;
            }
            throw;
        }
        for (int i = index; i < m_allocateMapSize - 1; i++)
            m_allocateMap[i] = m_allocateMap[i + 1];
        m_allocateMapSize--;
    }

private:
    using AllocInfo = std::pair<char*, size_t>;
    char* m_buffer;
    size_t m_bufferSize;
    size_t m_maxContinuousMemorySize = 0; // Only a size less than or equals to this MAY be allocated in this memory pool.
    bool m_maxContinuousMemorySizeValid = false;
    AllocInfo* m_allocateMap = nullptr;
    size_t m_allocateMapSize = 0;
    size_t m_allocateMapSizeAlloc = 0;
    MemoryPool* m_next = nullptr;

    void twoInsert(char* pos, size_t size)
    {
        int left, right;
        int middle, j, i = m_allocateMapSize;
        left = 0;
        right = i - 1;
        while (right >= left)
        {
            middle = (left + right) / 2;
            if (pos < m_allocateMap[middle].first)
                right = middle - 1;
            else
                left = middle + 1;
        }
        for (j = i - 1; j >= left; j--)
            m_allocateMap[j + 1] = m_allocateMap[j];
        m_allocateMap[left].first = pos;
        m_allocateMap[left].second = size;
    }

    int binarySearch(char* pos)
    {
        int start = 0;
        int end = m_allocateMapSize - 1;

        while (start <= end)
        {
            int mid = start + (end - start) / 2;
            if (m_allocateMap[mid].first == pos)
                return mid;

            if (pos < m_allocateMap[mid].first)
                end = mid - 1;
            else
                start = mid + 1;
        }
        return -1;
    }

    void insertAllocateMap(char* pos, size_t size)
    {
        if (m_allocateMapSize + 1 > m_allocateMapSizeAlloc)
        {
            m_allocateMap = static_cast<AllocInfo*>(realloc(m_allocateMap, m_allocateMapSizeAlloc * 2 * sizeof(AllocInfo)));
            m_allocateMapSizeAlloc *= 2;
        }

        twoInsert(pos, size);
        m_allocateMapSize++;
    }

};

template <class T, size_t Size>
class FixedMemoryPool
{
public:
    enum
    {
        PoolElumNum = Size, ElumSize = sizeof(T)
    };
    using ElemType = T;
    using AllocatorType = FixedAllocator<ElemType, Size>;

    FixedMemoryPool() :
        m_buffer(new ElemType[PoolElumNum]),
        m_allocateMap(new bool[PoolElumNum]),
        m_freeSize(new size_t(PoolElumNum))
    {
        memset(m_allocateMap, 0, sizeof(bool)*PoolElumNum);
    }
    FixedMemoryPool(const FixedMemoryPool& other)
    {
        m_buffer = other.m_buffer;
        m_allocateMap = other.m_allocateMap;
        m_next = other.m_next;
        m_freeSize = other.m_freeSize;;
    }
    FixedMemoryPool(FixedMemoryPool&&) = delete;
    FixedMemoryPool& operator = (const FixedMemoryPool&) = delete;

    ~FixedMemoryPool()
    {
        delete m_allocateMap;
        delete[] m_buffer;
        delete m_next;
    }

    // Allocate memory
    ElemType* allocate(std::size_t size)
    {
        if (size == 0) return nullptr;

        size_t pos = 0;
        bool notEnough = *m_freeSize == 0;

        if(!notEnough)
            for (; pos < PoolElumNum; ++pos)
                if (!m_allocateMap[pos]) break;

        notEnough = notEnough || pos == PoolElumNum;

        if (notEnough)
        {
            if (!m_next) m_next = new FixedMemoryPool<ElemType, PoolElumNum/* * 2*/>();
            return m_next->allocate(size);
        }

        m_allocateMap[pos] = true;
        return reinterpret_cast<ElemType*>(m_buffer + pos);
    }

    // Deallocate memory
    template <class U>
    void deallocate(U* ptr, std::size_t size)
    {
        size_t pos = (ptr - m_buffer) / ElumSize;
        if(pos<0||pos>=PoolElumNum)
        {
            if (m_next)
            {
                m_next->deallocate(ptr, size);
                return;
            }
            throw;
        }
        ++*m_freeSize;
        m_allocateMap[(ptr - m_buffer) / ElumSize] = false;
    }

private:
    ElemType* m_buffer;
    bool* m_allocateMap = nullptr; // false if it's free, and true if a object has already been here.
    size_t* m_freeSize;
    FixedMemoryPool<ElemType, PoolElumNum/* * 2*/>* m_next = nullptr;
};

template <class T, class U>
bool operator==(const Allocator<T>& a, const Allocator<U>& b)
{
    return (&a.getPool()) == (&b.getPool());
}
template <class T, class U>
bool operator!=(const Allocator<T>& a, const Allocator<U>& b)
{
    return !(a == b);
}
template <class T, class U, size_t Size1,size_t Size2>
bool operator==(const FixedAllocator<T, Size1>& a, const FixedAllocator<U, Size2>& b)
{
    return (&a.getPool()) == (&b.getPool());
}
template <class T, class U, size_t Size1, size_t Size2>
bool operator!=(const FixedAllocator<T, Size1>& a, const FixedAllocator<U, Size2>& b)
{
    return !(a == b);
}

#endif // MEMORYPOOL_H__
