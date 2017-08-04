#include "chipy/Dictionary.h"
#include "chipy/Tuple.h"

namespace chipy
{

DictItemIterator::DictItemIterator(MemoryManager &mem, Dictionary &dict)
    : Generator(mem), m_dict(dict), m_it(dict.elements().begin())
{
    m_dict.raise();
}

DictItemIterator::~DictItemIterator()
{
    m_dict.drop();
}

ValuePtr DictItemIterator::next() throw(stop_iteration_exception)
{
    if(m_it == m_dict.elements().end())
        throw stop_iteration_exception();

    auto key = new (memory_manager()) StringVal(memory_manager(), m_it->first);
    auto t = new (memory_manager()) Tuple(memory_manager(), key, m_it->second);
    m_it++;
    key->drop();
    return t;
}

Value* DictItemIterator::duplicate()
{
    return new (memory_manager()) DictItemIterator(memory_manager(), m_dict);
}

DictKeyIterator::DictKeyIterator(MemoryManager &mem, Dictionary &dict)
    : Generator(mem), m_dict(dict), m_it(dict.elements().begin())
{
    m_dict.raise();
}

DictKeyIterator::~DictKeyIterator()
{
    m_dict.drop();
}

DictItems* Dictionary::items()
{
    return new (memory_manager()) DictItems(memory_manager(), *this);
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
    return new (memory_manager()) DictKeyIterator(memory_manager(), m_dict);
}

Dictionary::~Dictionary()
{
    for(auto &it : m_elements)
        it.second->drop();
}

ValuePtr DictItems::duplicate()
{
    return new (memory_manager()) DictItems(memory_manager(), m_dict);
}

IteratorPtr DictItems::iterate()
{
    return new (memory_manager()) DictItemIterator(memory_manager(), m_dict);
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
    return new (memory_manager()) DictKeyIterator(memory_manager(), *this);
}

ValuePtr Dictionary::get(const std::string &key)
{
    auto it = m_elements.find(key);
    if(it == m_elements.end())
        return nullptr;

    it->second->raise();
    return it->second;
}

void Dictionary::insert(const std::string &key, ValuePtr value)
{
    auto it = m_elements.find(key);
    if(it != m_elements.end())
        it->second->drop();

    m_elements[key] = value;
    value->raise();
}

ValueType Dictionary::type() const
{
    return ValueType::Dictionary;
}

ValuePtr Dictionary::duplicate()
{
    auto d = new (memory_manager()) Dictionary(memory_manager());

    for(auto &it: m_elements)
    {
        it.second->raise();
        d->insert(it.first, it.second);
    }

    return d;
}

}
