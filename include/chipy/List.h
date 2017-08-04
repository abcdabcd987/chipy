#pragma once

#include "Iterator.h"

namespace chipy
{

class List;

class ListIterator : public Generator
{
public:
    ListIterator(MemoryManager& mem, List &list);
    ~ListIterator();

    ValuePtr next() throw(stop_iteration_exception) override;

    ValuePtr duplicate() override;

private:
    List &m_list;
    uint32_t m_pos;
};

class List : public IterateableValue
{
public:
    List(MemoryManager &mem)
        : IterateableValue(mem)
    {}

    ~List();

    IteratorPtr iterate() override;

    ValuePtr duplicate() override;

    ValuePtr get(uint32_t index);

    uint32_t size() const;

    bool contains(const Value &value) const;

    ValueType type() const override;

    void append(Value *val);

    const std::vector<Value*>& elements() const;

private:
    std::vector<Value*> m_elements;
};

}
