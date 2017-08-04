#pragma once

#include "Value.h"

namespace chipy
{

class Tuple : public Value
{
public:
    Tuple(MemoryManager &mem, Value *first, Value *second)
        : Value(mem), m_first(first), m_second(second)
    {
        m_first->raise();
        m_second->raise();
    }

    ~Tuple()
    {
        m_first->drop();
        m_second->drop();
    }

    ValueType type() const override
    {
        return ValueType::Tuple;
    }

    Value* duplicate() override
    {
        return new (memory_manager()) Tuple(memory_manager(), m_first, m_second);
    }

    Value* first()
    {
        return m_first;
    }

    Value* second()
    {
        return m_second;
    }

private:
    Value *m_first;
    Value *m_second;
};

}
