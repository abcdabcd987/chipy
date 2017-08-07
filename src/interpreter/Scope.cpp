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

TuplePtr Scope::create_tuple(ValuePtr first, ValuePtr second)
{
    return wrap_value<Tuple>(new (memory_manager()) Tuple(memory_manager(), first, second));
}

StringValPtr Scope::create_string(const std::string &str)
{
    return wrap_value<StringVal>(new (memory_manager()) StringVal(memory_manager(), str));
}

IntValPtr Scope::create_integer(const int32_t value)
{
    return wrap_value<IntVal>(new (memory_manager()) IntVal(memory_manager(), value));
}

BoolValPtr Scope::create_boolean(const bool value)
{
    return wrap_value<BoolVal>(new (memory_manager()) BoolVal(memory_manager(), value));
}

DictionaryPtr Scope::create_dictionary()
{
    return wrap_value<Dictionary>(new (memory_manager()) Dictionary(memory_manager()));
}

FloatValPtr Scope::create_float(const double& value)
{
    return wrap_value<FloatVal>(new (memory_manager()) FloatVal(memory_manager(), value));
}

ValuePtr Scope::create_none()
{
    return std::shared_ptr<Value>{ nullptr };
}

ListPtr Scope::create_list()
{
    return wrap_value<List>(new (memory_manager()) List(memory_manager()));
}

}


