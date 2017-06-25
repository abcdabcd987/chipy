#pragma once

#include <functional>
#include "chipy/Value.h"

namespace chipy
{
    
class Module : public Value
{
public:
    virtual ~Module() {}

    ValueType type() const override { return ValueType::Module; }

    virtual Value* get_member(const std::string& name) const = 0;

    Value* duplicate() const override
    {
        return nullptr; //not supported
    }
};

class Function : public Callable
{
public:
    Function(std::function<Value*(const std::vector<Value*>&)> func)
        : m_func(func)
    {}

    Value* duplicate() const override
    {
        return new Function(m_func);
    }

    Value* call(const std::vector<Value*>& args) override
    {
        return m_func(args);
    }

    ValueType type() const override { return ValueType::Function; }

private:
    const std::function<Value* (const std::vector<Value*>)> m_func;
};

class RandModule : public Module
{
public:
    Value* get_member(const std::string &name) const override;
};

extern RandModule g_module_rand;

}
