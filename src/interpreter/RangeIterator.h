#pragma once

#include "chipy/Iterator.h"

namespace chipy
{

class RangeIterator : public Generator
{
public:
    RangeIterator(MemoryManager &mem, int32_t start, int32_t end, int32_t step_size)
        : Generator(mem),
        m_start(start), m_end(end), m_step_size(step_size)
    {
        m_pos = m_start;
    }

    ValuePtr duplicate() override
    {
        return wrap_value(new (memory_manager()) RangeIterator(memory_manager(), m_start, m_end, m_step_size));
    }

    ValuePtr next() throw(stop_iteration_exception) override
    {
        if(m_pos >= m_end)
            throw stop_iteration_exception();

        auto res = wrap_value(new (memory_manager()) IntVal(memory_manager(), m_pos));
        m_pos += m_step_size;

        return res;
    }

private:
    int32_t m_pos;
    const int32_t m_start, m_end, m_step_size;
};

}

