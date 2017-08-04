#pragma once

#include "Value.h"

namespace chipy
{

class Callable : public Value
{
protected:
    Callable(MemoryManager &mem)
        :  Value(mem)
    {}
    
public:
    virtual ValuePtr call(const std::vector<ValuePtr>& args) = 0;

    bool is_callable() const override
    {
        return true;
    }
};

}
