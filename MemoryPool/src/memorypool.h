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
    MemoryPool(MemoryPool&&) = delete;
    MemoryPool& operator = (const MemoryPool&) = delete;
    ~MemoryPool() { delete[] m_buffer; delete m_next; }

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
    T* allocate(std::size_t n, MemoryPool** pool)
    {
        if (n == 0) return nullptr;

        bool notEnough = false; // If need create a new memory pool
        char* pos = m_buffer;
        if((n>m_maxContinuousMemorySize&&m_maxContinuousMemorySizeValid)||n>m_bufferSize - m_bufferUsedSize)
        {
            notEnough = true;
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
                            notEnough = true;
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
        if(notEnough)
        {
            if (!m_next)
            {
                size_t size = m_bufferSize;
                while (n > m_bufferSize) m_bufferSize *= 2;
                m_next = new MemoryPool(size);
            }
            m_maxContinuousMemorySize = n > m_maxContinuousMemorySize ? m_maxContinuousMemorySize : n - 1;
            return m_next->allocate<T>(n, pool);
        }
        m_allocateMap.insert({ pos ,n });
        m_bufferUsedSize += n;
        *pool = this;
        return reinterpret_cast<T*>(pos);
    }

    // Deallocate memory
    template <class T>
    void deallocate(T* p, std::size_t n)
    {
        m_maxContinuousMemorySizeValid = false; // Make it invalid
        auto iter = m_allocateMap.find({ reinterpret_cast<char*>(p),n });
        if (iter == m_allocateMap.cend()) throw;
        m_allocateMap.erase(iter);
    }

private:
    using AllocInfo = std::pair<char*, size_t>;
    char* m_buffer;
    size_t m_bufferSize;
    size_t m_bufferUsedSize;
    size_t m_maxContinuousMemorySize; // Only a size less than or equals to this MAY be allocated in this memory pool.
    bool m_maxContinuousMemorySizeValid = false;
    std::set<AllocInfo> m_allocateMap;
    MemoryPool* m_next = nullptr;
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
