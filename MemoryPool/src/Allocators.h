#ifndef ALLOCATORS_H__
#define ALLOCATORS_H__
class MemoryPool;
template <class T, size_t Size>
class FixedMemoryPool;

// Allocators for memory pools
template <class T>
class Allocator
{
public:
    using value_type = T;
    using pointer = T*;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;

    using PoolType = MemoryPool;

    template <class U>
    struct rebind { typedef Allocator<U> other; };

    Allocator(PoolType* pool) :m_pool(pool) {}
    template <class U> Allocator(Allocator<U>& other) : m_pool(other.getPool()) {}

    pointer allocate(std::size_t n)
    {
        return m_pool->allocate<value_type>(n*sizeof(value_type));
    }
    void deallocate(pointer p, std::size_t n)
    {
        m_pool->deallocate(p, n*sizeof(value_type));
    }


    template <class... Args>
    void construct(pointer ptr, Args&&... args)
    {
        new(ptr) value_type(std::forward<Args>(args)...);
    }

    void destory(pointer ptr)
    {
        ptr->~value_type();
    }

    template <class... Args>
    pointer newobj(Args&&... args)
    {
        auto ptr = allocate(1);
        construct(ptr, std::forward<Args>(args)...);
        return ptr;
    }

    void delobj(pointer ptr)
    {
        destory(ptr);
        deallocate(ptr, 1);
    }


    PoolType* getPool() { return m_pool; }

private:

    PoolType* m_pool;
};

template <class T, size_t Size>
class FixedAllocator
{
public:
    using value_type = T;
    using pointer = T*;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;

    using PoolType = FixedMemoryPool<T, Size>;

    template <class U>
    struct rebind { typedef FixedAllocator<U, Size> other; };

    FixedAllocator() = default;
    template <class U> FixedAllocator(FixedAllocator<U, Size>&) {}

    pointer allocate(std::size_t n)
    {
        return m_pool.allocate(n);
    }
    void deallocate(pointer p, std::size_t n)
    {
        m_pool.deallocate(p, n);
    }

    template <class... Args>
    void construct(pointer ptr, Args&&... args)
    {
        new(ptr) value_type(std::forward<Args>(args)...);
    }

    void destory(pointer ptr)
    {
        ptr->~value_type();
    }

    template <class... Args>
    pointer newobj(Args&&... args)
    {
        auto ptr = allocate(1);
        construct(ptr, std::forward<Args>(args)...);
        return ptr;
    }

    void delobj(pointer ptr)
    {
        destory(ptr);
        deallocate(ptr, 1);
    }


    PoolType* getPool() { return m_pool; }

private:
    PoolType m_pool;

};


#endif // ALLOCATORS_H__
