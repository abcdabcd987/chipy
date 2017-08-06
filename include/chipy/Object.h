#pragma once

#include <unordered_map>
#include <vector>

namespace chipy
{

class MemoryManager
{
public:
    MemoryManager();

    MemoryManager(MemoryManager &other) = delete;

    void* malloc(size_t sz);
    void free(void* ptr);

private:
    std::vector<uint8_t> m_buffer;
    size_t   m_buffer_pos;

    struct AllocInfo
    {
        size_t size;
    };

    void resize(size_t new_size);

    std::unordered_map<intptr_t, AllocInfo> m_allocs;
};

class Object
{
public:
    virtual ~Object() {}

    static void* operator new(std::size_t sz, MemoryManager &mem_mgr)
    {
        auto ptr = mem_mgr.malloc(sz);
        return ptr;
    }

    static void operator delete(void *ptr)
    {
        auto obj = reinterpret_cast<Object*>(ptr);
        obj->m_mem.free(ptr);
    }

    MemoryManager& memory_manager()
    {
        return m_mem;
    }

protected:
    Object(MemoryManager &mem)
        : m_mem(mem)
    {}

    MemoryManager &m_mem;
};

}
