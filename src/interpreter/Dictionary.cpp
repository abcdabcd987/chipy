#include "chipy/Dictionary.h"
#include "chipy/Tuple.h"

namespace chipy
{

DictItemIterator::DictItemIterator(MemoryManager &mem, Dictionary &dict)
    : Generator(mem), m_dict(dict), m_it(dict.elements().begin())
{
}

ValuePtr DictItemIterator::next() throw(stop_iteration_exception)
{
    if(m_it == m_dict.elements().end())
        throw stop_iteration_exception();

    auto key = wrap_value(new (memory_manager()) StringVal(memory_manager(), m_it->first));
    auto t = wrap_value(new (memory_manager()) Tuple(memory_manager(), key, m_it->second));
    m_it++;
    return t;
}

ValuePtr DictItemIterator::duplicate()
{
    return wrap_value(new (memory_manager()) DictItemIterator(memory_manager(), m_dict));
}

DictKeyIterator::DictKeyIterator(MemoryManager &mem, Dictionary &dict)
    : Generator(mem), m_dict(dict), m_it(dict.elements().begin())
{
}

DictItemsPtr Dictionary::items()
{
    return wrap_value(new (memory_manager()) DictItems(memory_manager(), *this));
}

ValuePtr DictKeyIterator::next() throw(stop_iteration_exception)
{
    if(m_it == m_dict.elements().end())
        throw stop_iteration_exception();

    // FIXME implement tuples
    auto &elem = m_it->second;
    m_it++;
    return elem;
}

ValuePtr DictKeyIterator::duplicate()
{
    return wrap_value(new (memory_manager()) DictKeyIterator(memory_manager(), m_dict));
}

ValuePtr DictItems::duplicate()
{
    return wrap_value(new (memory_manager()) DictItems(memory_manager(), m_dict));
}

IteratorPtr DictItems::iterate()
{
    return wrap_value(new (memory_manager()) DictItemIterator(memory_manager(), m_dict));
}

const std::map<std::string, ValuePtr>& Dictionary::elements() const
{
    return m_elements;
}

std::map<std::string, ValuePtr>& Dictionary::elements()
{
    return m_elements;
}

uint32_t Dictionary::size() const
{
    return m_elements.size();
}

IteratorPtr Dictionary::iterate()
{
    return wrap_value(new (memory_manager()) DictKeyIterator(memory_manager(), *this));
}

ValuePtr Dictionary::get(const std::string &key)
{
    auto it = m_elements.find(key);
    if(it == m_elements.end())
        return nullptr;

    return it->second;
}

void Dictionary::insert(const std::string &key, ValuePtr value)
{
    m_elements[key] = value;
}

ValueType Dictionary::type() const
{
    return ValueType::Dictionary;
}

ValuePtr Dictionary::duplicate()
{
    auto d = wrap_value(new (memory_manager()) Dictionary(memory_manager()));

    for(auto &it: m_elements)
    {
        d->insert(it.first, it.second);
    }

    return d;
}

}
