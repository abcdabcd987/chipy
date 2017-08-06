#pragma once

#include "Value.h"

namespace chipy
{

class Tuple : public Value
{
public:
    Tuple(MemoryManager &mem, ValuePtr first, ValuePtr second)
        : Value(mem), m_first(first), m_second(second)
    {
    }

    ValueType type() const override
    {
        return ValueType::Tuple;
    }

    ValuePtr duplicate() override
    {
        return ValuePtr(new (memory_manager()) Tuple(memory_manager(), m_first, m_second));
    }

    ValuePtr first()
    {
        return m_first;
    }

    ValuePtr second()
    {
        return m_second;
    }

private:
    ValuePtr m_first;
    ValuePtr m_second;
};

typedef std::shared_ptr<Tuple> TuplePtr;

}
