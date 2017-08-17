#pragma once

#include "Iterator.h"

namespace chipy
{

class List;
typedef std::shared_ptr<List> ListPtr;

class ListIterator : public Generator
{
public:
    ListIterator(MemoryManager& mem, List &list);

    ValuePtr next() override;

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

    IteratorPtr iterate() override;

    ValuePtr duplicate() override;

    ValuePtr get(uint32_t index);

    uint32_t size() const;

    bool contains(const Value &value) const;

    ValueType type() const override;

    void append(ValuePtr val);

    const std::vector<ValuePtr>& elements() const;

private:
    std::vector<ValuePtr> m_elements;
};

}
