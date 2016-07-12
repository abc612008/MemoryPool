#ifndef MEMORYPOOL_H__
#define MEMORYPOOL_H__
#include <cstddef>

class MemoryPool
{
public:
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
            return (pointer)malloc(n*sizeof(T));
        }
        void deallocate(T* p, std::size_t n)
        {
            free(p);
        }

        MemoryPool& getPool() { return m_pool; }

    private:
        MemoryPool& m_pool;
    };


    template <class T>
    Allocator<T> getAllocator()
    {
        return Allocator<T>(*this);
    }


};


template <class T, class U>
bool operator==(const MemoryPool::Allocator<T>& a, const MemoryPool::Allocator<U>& b)
{

}
template <class T, class U>
bool operator!=(const MemoryPool::Allocator<T>& a, const MemoryPool::Allocator<U>& b)
{
    return !(a == b);
}


#endif // MEMORYPOOL_H__
