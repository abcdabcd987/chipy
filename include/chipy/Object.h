#pragma once

#include <unordered_map>

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
    uint8_t *data;
    size_t size;
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
