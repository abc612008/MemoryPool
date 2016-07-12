#ifndef MEMORYPOOL_H__
#define MEMORYPOOL_H__
#include <set>

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

    // Allocators for memory pools
    template <class T>
    class Allocator
    {
    public:
        using value_type = T;
        using pointer = T*;

        Allocator(MemoryPool* pool) :m_pool(pool) {}
        template <class U> Allocator(Allocator<U>& other): m_pool(other.getPool()) {}

        pointer allocate(std::size_t n)
        {
            return m_pool->allocate<value_type>(n*sizeof(value_type), &m_pool);
        }
        void deallocate(pointer p, std::size_t n)
        {
            m_pool->deallocate(p, n);
        }

        MemoryPool* getPool() { return m_pool; }

    private:
        MemoryPool* m_pool;
    };

    // Get an allocator
    template <class T>
    Allocator<T> getAllocator()
    {
        return Allocator<T>(this);
    }

    // Allocate memory
    template <class T>
    T* allocate(std::size_t size, MemoryPool** pool)
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
                    auto ptr = ++m_allocateMap;
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
                size_t newSize = m_bufferSize;
                while (size > m_bufferSize) m_bufferSize *= 2;
                m_next = new MemoryPool(newSize);
            }
            m_maxContinuousMemorySize = size > m_maxContinuousMemorySize ? m_maxContinuousMemorySize : size - 1;
            return m_next->allocate<T>(size, pool);
        }
        insertAllocaateMap(pos, size);
        *pool = this;
        return reinterpret_cast<T*>(pos);
    }

    // Deallocate memory
    template <class T>
    void deallocate(T* p, std::size_t size)
    {
        m_maxContinuousMemorySizeValid = false; // Make it invalid
        int index = binarySearch(reinterpret_cast<char*>(p));
        if (index == -1|| m_allocateMap[index].second!= size) throw;
        for (int j = m_allocateMapSize - 1; j >= index; j--)
            m_allocateMap[j + 1] = m_allocateMap[j];
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

    void insertAllocaateMap(char* pos, size_t size)
    {
        if (m_allocateMapSize + 1 > m_allocateMapSizeAlloc)
        {
            m_allocateMap = static_cast<AllocInfo*>(realloc(m_allocateMap, m_allocateMapSizeAlloc * 2));
            m_allocateMapSizeAlloc *= 2;
        }

        twoInsert(pos, size);
        m_allocateMapSize++;
    }

};


template <class T, class U>
bool operator==(const MemoryPool::Allocator<T>& a, const MemoryPool::Allocator<U>& b)
{
    return (&a.getPool()) == (&b.getPool());
}
template <class T, class U>
bool operator!=(const MemoryPool::Allocator<T>& a, const MemoryPool::Allocator<U>& b)
{
    return !(a == b);
}

#endif // MEMORYPOOL_H__
