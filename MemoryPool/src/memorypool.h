#ifndef MEMORYPOOL_H__
#define MEMORYPOOL_H__
#include <set>

class MemoryPool
{
public:
    MemoryPool(int unitPoolSize) :m_bufferSize(unitPoolSize)
    {
        m_buffer = new char[m_bufferSize];
    }
    MemoryPool(const MemoryPool&) = delete;
    MemoryPool& operator = (const MemoryPool&) = delete;
    ~MemoryPool() { delete[] m_buffer; }

    // Allocators for memory pools
    template <class T>
    class Allocator
    {
    public:
        using value_type = T;
        using pointer = T*;

        Allocator(MemoryPool& pool) :m_pool(pool) {}
        template <class U> Allocator(Allocator<U>& other): m_pool(other.getPool()) {}

        pointer allocate(std::size_t n)
        {
            return m_pool.allocate<value_type>(n*sizeof(value_type));
        }
        void deallocate(pointer p, std::size_t n)
        {
            m_pool.deallocate(p, n);
        }

        MemoryPool& getPool() { return m_pool; }

    private:
        MemoryPool& m_pool;
    };

    // Get an allocator
    template <class T>
    Allocator<T> getAllocator()
    {
        return Allocator<T>(*this);
    }

    // Allocate memory
    template <class T>
    T* allocate(std::size_t n)
    {
        if (n == 0) return nullptr;

        bool needCreate = false; // If need create a new memory pool
        char* pos = m_buffer;
        if(n>m_maxContinuousMemorySize&&m_maxContinuousMemorySize!=-1)
        {
            needCreate = true;
        }
        if(n>m_bufferSize-m_bufferUsedSize)
        {
            needCreate = true;
            m_maxContinuousMemorySize = n - 1;
        }
        else
        {
            if (m_allocateMap.size() > 0)
            {
                if (m_allocateMap.size() > 1)
                {
                    const AllocInfo* lastInfo = &*m_allocateMap.cbegin();
                    auto iter = ++m_allocateMap.cbegin();
                    for (; iter != m_allocateMap.cend(); ++iter)
                    {
                        if (lastInfo->first + lastInfo->second + n < iter->first)
                        {
                            pos = lastInfo->first + lastInfo->second;
                            break;
                        }
                        lastInfo = &*iter;
                    }
                    if (iter == m_allocateMap.cend())
                    {
                        if (lastInfo->first + lastInfo->second + n < m_buffer + m_bufferSize)
                        {
                            pos = lastInfo->first + lastInfo->second;
                        }
                        else
                        {
                            needCreate = true;
                            m_maxContinuousMemorySize = n;
                        }
                    }
                }
                else
                {
                    pos += m_allocateMap.cbegin()->second;
                }
            }
        }
        if(needCreate)
        {
            return nullptr; //TODO: create a new memory pool, and allocate memory from it.
        }
        m_allocateMap.insert({ pos ,n });
        m_bufferUsedSize += n;
        return reinterpret_cast<T*>(pos);
    }

    // Deallocate memory
    template <class T>
    void deallocate(T* p, std::size_t n)
    {
        m_maxContinuousMemorySize = -1;
    }

private:
    using AllocInfo = std::pair<char*, size_t>;
    char* m_buffer;
    size_t m_bufferSize;
    size_t m_bufferUsedSize;
    size_t m_maxContinuousMemorySize=-1; // Only a size less than or equals to this MAY be allocated in this memory pool.
    std::set<AllocInfo> m_allocateMap;
};


template <class T, class U>
bool operator==(const MemoryPool::Allocator<T>& a, const MemoryPool::Allocator<U>& b)
{
    return a.getPool() == b.getPool();
}
template <class T, class U>
bool operator!=(const MemoryPool::Allocator<T>& a, const MemoryPool::Allocator<U>& b)
{
    return !(a == b);
}


#endif // MEMORYPOOL_H__
