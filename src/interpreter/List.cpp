#include "chipy/List.h"

namespace chipy
{

List::~List()
{
    for(auto elem : m_elements)
        elem->drop();
}

IteratorPtr List::iterate()
{
    return new (memory_manager()) ListIterator(memory_manager(), *this);
}

ValuePtr List::duplicate()
{
    auto d = new (memory_manager()) List(memory_manager());

    for(auto elem: m_elements)
    {
        elem->raise();
        d->append(elem);
    }

    return d;
}

Value* List::get(uint32_t index)
{
    if(index >= size())
        throw std::runtime_error("List index out of range");

    auto elem = m_elements[index];
    elem->raise();
    return elem;
}

uint32_t List::size() const
{
    return m_elements.size();
}

const std::vector<Value*>& List::elements() const
{
    return m_elements;
}

bool List::contains(const Value &value) const
{
    for(auto elem: m_elements)
    {
        if(*elem == value)
            return true;
    }

    return false;
}

ValueType List::type() const
{
    return ValueType::List;
}

void List::append(Value *val)
{
    val->raise();
    m_elements.push_back(val);
}

ListIterator::ListIterator(MemoryManager &mem, List &list)
    : Generator(mem), m_list(list), m_pos(0)
{
    m_list.raise();
}

ListIterator::~ListIterator()
{
    m_list.drop();
}

ValuePtr ListIterator::next() throw(stop_iteration_exception)
{
    if(m_pos >= m_list.size())
        throw stop_iteration_exception();

    auto res = m_list.get(m_pos);
    m_pos += 1;

    return res;
}

ValuePtr ListIterator::duplicate()
{
    return new (memory_manager()) ListIterator(memory_manager(), m_list);
}

}
