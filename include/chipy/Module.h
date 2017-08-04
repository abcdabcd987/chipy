#pragma once

#include <functional>
#include "chipy/Value.h"
#include "chipy/Callable.h"

namespace chipy
{
    
class Module : public Value
{
public:
    Module(MemoryManager &mem)
        : Value(mem)
    {}

    virtual ~Module() {}

    ValueType type() const override { return ValueType::Module; }

    virtual ValuePtr get_member(const std::string& name) = 0;

    ValuePtr duplicate() override
    {
        return nullptr; //not supported
    }
};

class Function : public Callable
{
public:
    Function(MemoryManager &mem, std::function<ValuePtr(const std::vector<ValuePtr>&)> func)
        : Callable(mem), m_func(func)
    {}

    ValuePtr duplicate() override
    {
        return new (memory_manager()) Function(memory_manager(), m_func);
    }

    ValuePtr call(const std::vector<Value*>& args) override
    {
        return m_func(args);
    }

    ValueType type() const override { return ValueType::Function; }

private:
    const std::function<ValuePtr (const std::vector<ValuePtr>)> m_func;
};

typedef Module* ModulePtr;

}
