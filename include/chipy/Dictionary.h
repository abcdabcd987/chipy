#pragma once

#include "Value.h"
#include "Iterator.h"
#include "Callable.h"

namespace chipy
{

class Dictionary;

class DictItemIterator : public Generator
{
public:
    DictItemIterator(MemoryManager &mem, Dictionary &dict);
    ~DictItemIterator();

    ValuePtr next() throw(stop_iteration_exception) override;

    ValuePtr duplicate() override;

private:
    Dictionary &m_dict;
    std::map<std::string, Value*>::iterator m_it;
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

    ValuePtr call(const std::vector<Value*>& args) override
    {
        if(args.size() != 0)
            throw std::runtime_error("invalid number of arguments");

        return iterate();
    }

    ValuePtr duplicate() override;

    Iterator* iterate();

//    uint32_t size() const override;

private:
    Dictionary &m_dict;
};

class DictKeyIterator : public Generator
{
public:
    DictKeyIterator(MemoryManager &mem, Dictionary &dict);
    ~DictKeyIterator();

    Value* next() throw(stop_iteration_exception) override;

    ValuePtr duplicate() override;

private:
    Dictionary &m_dict;
    std::map<std::string, Value*>::iterator m_it;
};

class Dictionary : public IterateableValue
{
public:
    using IterateableValue::IterateableValue;
    ~Dictionary();

    Iterator* iterate();

    Value* get(const std::string &key);

    DictItems *items();

    uint32_t size() const;

    void insert(const std::string &key, Value *value);

    ValueType type() const override;

    ValuePtr duplicate() override;

    std::map<std::string, ValuePtr>& elements();

    const std::map<std::string, ValuePtr>& elements() const;

private:
    std::map<std::string, ValuePtr> m_elements;
};

}
