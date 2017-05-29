#include "chipy/Value.h"

namespace chipy
{

DictItemIterator::DictItemIterator(Dictionary &dict)
    : m_dict(dict), m_it(dict.elements().begin())
{
    m_dict.raise();
}

DictItemIterator::~DictItemIterator()
{
    m_dict.drop();
}

Value* DictItemIterator::next() throw(stop_iteration_exception)
{
    if(m_it == m_dict.elements().end())
        throw stop_iteration_exception();

    auto key = create_string(m_it->first);
    auto t = create_tuple(key, m_it->second);
    m_it++;
    key->drop();
    return t;
}

Value* DictItemIterator::duplicate() const
{
    return new DictItemIterator(m_dict);
}



DictKeyIterator::DictKeyIterator(Dictionary &dict)
    : m_dict(dict), m_it(dict.elements().begin())
{
    m_dict.raise();
}

DictKeyIterator::~DictKeyIterator()
{
    m_dict.drop();
}

DictItems* Dictionary::items()
{
    return new DictItems(*this);
}

Value* DictKeyIterator::next() throw(stop_iteration_exception)
{
    if(m_it == m_dict.elements().end())
        throw stop_iteration_exception();

    // FIXME implement tuples
    auto &elem = m_it->second;
    m_it++;
    return elem;
}

Value* DictKeyIterator::duplicate() const
{
    return new DictKeyIterator(m_dict);
}

Dictionary::Dictionary()
{}

Dictionary::~Dictionary()
{
    for(auto &it : m_elements)
        it.second->drop();
}

Value* DictItems::duplicate() const
{
    return new DictItems(m_dict);
}

Iterator* DictItems::iterate()
{
    return new DictItemIterator(m_dict);
}



std::map<std::string, Value*>& Dictionary::elements()
{
    return m_elements;
}

uint32_t Dictionary::size() const
{
    return m_elements.size();
}

Iterator* Dictionary::iterate()
{
    return new DictKeyIterator(*this);
}

Value* Dictionary::get(const std::string &key)
{
    auto it = m_elements.find(key);
    if(it == m_elements.end())
        return nullptr;

    it->second->raise();
    return it->second;
}

void Dictionary::insert(const std::string &key, Value *value)
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

Value* Dictionary::duplicate() const
{
    auto d = new Dictionary();

    for(auto &it: m_elements)
    {
        it.second->raise();
        d->insert(it.first, it.second);
    }

    return d;
}

}
