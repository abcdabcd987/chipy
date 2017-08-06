#pragma once

#include "Value.h"
#include "Iterator.h"
#include "Callable.h"

namespace chipy
{

class Dictionary;
typedef std::shared_ptr<Dictionary> DictionaryPtr;

class DictItemIterator : public Generator
{
public:
    DictItemIterator(MemoryManager &mem, Dictionary &dict);

    ValuePtr next() throw(stop_iteration_exception) override;

    ValuePtr duplicate() override;

private:
    Dictionary &m_dict;
    std::map<std::string, ValuePtr>::iterator m_it;
};

class DictItems : public Callable //public IterateableValue, public Callable
{
public:
   DictItems(MemoryManager &mem, Dictionary &dict)
        : Callable(mem), m_dict(dict)
    {}

    ValueType type() const override
    {
        return ValueType::DictItems;
    }

    ValuePtr call(const std::vector<ValuePtr>& args) override
    {
        if(args.size() != 0)
            throw std::runtime_error("invalid number of arguments");

        return iterate();
    }

    ValuePtr duplicate() override;

    IteratorPtr iterate();

//    uint32_t size() const override;

private:
    Dictionary &m_dict;
};

typedef std::shared_ptr<DictItems> DictItemsPtr;

class DictKeyIterator : public Generator
{
public:
    DictKeyIterator(MemoryManager &mem, Dictionary &dict);

    ValuePtr next() throw(stop_iteration_exception) override;

    ValuePtr duplicate() override;

private:
    Dictionary &m_dict;
    std::map<std::string, ValuePtr>::iterator m_it;
};

class Dictionary : public IterateableValue
{
public:
    Dictionary(MemoryManager &mem)
        : IterateableValue(mem)
    {}

    IteratorPtr iterate() override;

    ValuePtr get(const std::string &key);

    DictItemsPtr items();

    uint32_t size() const;

    void insert(const std::string &key, ValuePtr value);

    ValueType type() const override;

    ValuePtr duplicate() override;

    std::map<std::string, ValuePtr>& elements();

    const std::map<std::string, ValuePtr>& elements() const;

private:
    std::map<std::string, ValuePtr> m_elements;
};

}
