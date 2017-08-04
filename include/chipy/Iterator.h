#pragma once

#include "Value.h"

namespace chipy
{

class stop_iteration_exception {};

class Iterator : public Value
{
public:
    virtual ~Iterator() {}

    bool is_generator() const override
    {
        return false;
    }

    virtual Value* next() throw(stop_iteration_exception) = 0;

    ValueType type() const override
    {
        return ValueType::Iterator;
    }

protected:
    using Value::Value;
};

typedef Iterator* IteratorPtr;

class Generator : public Iterator
{
public:
    bool is_generator() const
    {
        return true;
    }

protected:
    using Iterator::Iterator;
};

class IterateableValue : public Value
{
public:
    virtual ~IterateableValue() {}

    bool can_iterate() const override
    {
        return true;
    }

    virtual uint32_t size() const = 0;

    virtual Iterator* iterate() = 0;

protected:
    using Value::Value;
};


}

