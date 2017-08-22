#include <chipy/Scope.h>
#include <chipy/Dictionary.h>
#include <chipy/List.h>
#include "Builtin.h"

namespace chipy
{

void Scope::set_value(const std::string &id, ValuePtr value)
{
    if(m_parent && m_parent->has_value(id))
    {
        m_parent->set_value(id, value);
        return;
    }

    // FIXME actually update references...
    auto it = m_values.find(id);
    if(it != m_values.end())
    {
        m_values.erase(it);
    }

    m_values[id] = value;
}

bool Scope::has_value(const std::string &id) const
{
    if(m_parent && m_parent->has_value(id))
        return true;

    return m_values.find(id) != m_values.end();
}

ValuePtr Scope::get_value(const std::string &id)
{
    Value* val = nullptr;

    if(id == BUILTIN_STR_NONE)
        return std::shared_ptr<Value>{nullptr};
    else if(id == BUILTIN_STR_RANGE)
        val = new (memory_manager()) Builtin(memory_manager(), BuiltinType::Range);
    else if(id == BUILTIN_STR_MAKE_INT)
        val = new (memory_manager()) Builtin(memory_manager(), BuiltinType::MakeInt);
    else if(id == BUILTIN_STR_MAKE_STR)
        val = new (memory_manager()) Builtin(memory_manager(), BuiltinType::MakeString);
    else if(id == BUILTIN_STR_PRINT)
        val = new (memory_manager()) Builtin(memory_manager(), BuiltinType::Print);

    if(val)
        return std::shared_ptr<Value>(val);

    auto it = m_values.find(id);
    if(it == m_values.end())
    {
        if(m_parent)
            return m_parent->get_value(id);
        else
            throw std::runtime_error("No such value: " + id);
    }

    return it->second;
}

void Scope::terminate()
{
    m_terminated = true;
}

bool Scope::is_terminated() const
{
    return m_terminated;
}



}


