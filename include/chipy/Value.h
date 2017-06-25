#pragma once

#include <stdint.h>
#include <string>
#include "json/json.h"

namespace chipy
{

enum class ValueType
{
    Bool,
    String,
    Integer,
    Float,
    DictItems,
    CppObject,
    Attribute,
    Builtin,
    List,
    Dictionary,
    Iterator,
    Tuple,
    Alias,
    Module,
    Function
};

class Value
{
public:
    virtual ValueType type() const = 0;

    virtual Value* duplicate() const = 0;

    void raise()
    {
        m_reference_count += 1;
    }

    virtual bool is_generator() const
    {
        return false;
    }

    virtual bool can_iterate() const
    {
        return false;
    }

    virtual bool is_callable() const
    {
        return false;
    }

    virtual bool bool_test() const
    {
        return true;
    }

    void drop()
    {
        m_reference_count -= 1;

        if(m_reference_count == 0)
            delete this;
    }

    virtual ~Value() {}

private:
    uint16_t m_reference_count = 1;
};

bool operator==(const Value &v1, const Value& v2);

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
};

class Tuple : public Value
{
public:
    Tuple(Value *first, Value *second)
        : m_first(first), m_second(second)
    {
        m_first->raise();
        m_second->raise();
    }

    ~Tuple()
    {
        m_first->drop();
        m_second->drop();
    }

    ValueType type() const override
    {
        return ValueType::Tuple;
    }

    Value* duplicate() const
    {
        return new Tuple(m_first, m_second);
    }

    Value* first()
    {
        return m_first;
    }

    Value* second()
    {
        return m_second;
    }

private:
    Value *m_first;
    Value *m_second;
};

template<typename value_type, ValueType value_type_val>
class PlainValue : public Value
{
public:
    PlainValue(value_type val)
        : m_value(val)
    {}

    ValueType type() const override
    {
        return value_type_val;
    }

    const value_type& get() const
    {
        return m_value;
    }

    void set(const value_type &v)
    {
        m_value = v;
    }

protected:
    value_type m_value;
};

class Alias : public Value
{
public:
    Alias(const std::string& name, const std::string &as_name)
        : m_name(name), m_as_name(as_name)
    {}

    ValueType type() const override
    {
        return ValueType::Alias;
    }

    Value* duplicate() const
    {
        return new Alias(name(), as_name());
    }

    const std::string name() const { return m_name; }
    const std::string as_name() const { return m_as_name; }

private:
    const std::string m_name, m_as_name;
};

class BoolVal : public PlainValue<bool, ValueType::Bool>
{
public:
    BoolVal(bool val) : PlainValue(val) {}
    Value* duplicate() const { return new BoolVal(m_value); }

    bool bool_test() const override { return m_value; }
};

class StringVal : public PlainValue<std::string, ValueType::String>
{
public:
    StringVal(const std::string &val) : PlainValue(val) {}
    Value* duplicate() const { return new StringVal(m_value); }
};

class FloatVal : public PlainValue<double, ValueType::Float>
{
public:
    FloatVal(const double &val) : PlainValue(val) {}
    Value* duplicate() const { return new FloatVal(m_value); }
};

class IntVal : public PlainValue<int32_t, ValueType::Integer>
{
public:
    IntVal(const int32_t &val) : PlainValue(val) {}
    Value* duplicate() const { return new IntVal(m_value); }

    bool bool_test() const override { return m_value != 0; }
};

class value_exception {};

template<typename T>
T* value_cast(Value *val)
{
    auto res = dynamic_cast<T*>(val);
    if(res == nullptr)
        throw value_exception();

    return res;
}


inline bool operator>(const Value &first, const Value &second)
{
    if(first.type() == ValueType::Integer && second.type() == ValueType::Integer)
    {
        return dynamic_cast<const IntVal&>(first).get() > dynamic_cast<const IntVal&>(second).get();
    }
    else
        return false;
}

inline bool operator>=(const Value &first, const Value &second)
{
    if(first.type() == ValueType::Integer && second.type() == ValueType::Integer)
    {
        return dynamic_cast<const IntVal&>(first).get() >= dynamic_cast<const IntVal&>(second).get();
    }
    else
        return false;
}

inline bool operator==(const Value &first, const Value &second)
{
    if(first.type() == ValueType::String && second.type() == ValueType::String)
    {
        return dynamic_cast<const StringVal&>(first).get() == dynamic_cast<const StringVal&>(second).get();
    }
    else if(first.type() == ValueType::Integer && second.type() == ValueType::Integer)
    {
        return dynamic_cast<const IntVal&>(first).get() == dynamic_cast<const IntVal&>(second).get();
    }
    else
        return false;
}

class Generator : public Iterator
{
public:
    bool is_generator() const
    {
        return true;
    }
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
};

class Dictionary;
class List;


class Callable : public Value
{
public:
    virtual Value* call(const std::vector<Value*>& args) = 0;

    bool is_callable() const override
    {
        return true;
    }
};

class DictItemIterator : public Generator
{
public:
    DictItemIterator(Dictionary &dict);
    ~DictItemIterator();

    Value* next() throw(stop_iteration_exception) override;

    Value* duplicate() const override;

private:
    Dictionary &m_dict;
    std::map<std::string, Value*>::iterator m_it;
};

class DictItems : public Callable //public IterateableValue, public Callable
{
public:
    DictItems(Dictionary &dict)
        : m_dict(dict)
    {}

    ValueType type() const override
    {
        return ValueType::DictItems;
    }

    Value* call(const std::vector<Value*>& args) override
    {
        if(args.size() != 0)
            throw std::runtime_error("invalid number of arguments");

        return iterate();
    }

    Value* duplicate() const override;

    Iterator* iterate();

//    uint32_t size() const override;

private:
    Dictionary &m_dict;
};

class DictKeyIterator : public Generator
{
public:
    DictKeyIterator(Dictionary &dict);
    ~DictKeyIterator();

    Value* next() throw(stop_iteration_exception) override;

    Value* duplicate() const override;

private:
    Dictionary &m_dict;
    std::map<std::string, Value*>::iterator m_it;
};

class CppObject
{
public:
    virtual ~CppObject() {}

    virtual Value* call_function(const std::string &func_name, const std::vector<Value*> &args) = 0;
};

class Dictionary : public IterateableValue
{
public:
    Dictionary();
    ~Dictionary();

    Iterator* iterate();

    Value* get(const std::string &key);

    DictItems *items();

    uint32_t size() const;

    void insert(const std::string &key, Value *value);

    ValueType type() const override;

    Value* duplicate() const override;

    std::map<std::string, Value*>& elements();

    const std::map<std::string, Value*>& elements() const;

private:
    std::map<std::string, Value*> m_elements;
};

class ListIterator : public Generator
{
public:
    ListIterator(List &list);
    ~ListIterator();

    Value* next() throw(stop_iteration_exception) override;

    Value* duplicate() const override;

private:
    List &m_list;
    uint32_t m_pos;
};

class List : public IterateableValue
{
public:
    ~List();

    Iterator* iterate() override;

    Value* duplicate() const override;

    Value* get(uint32_t index);

    uint32_t size() const;

    bool contains(const Value &value) const;

    ValueType type() const override;

    void append(Value *val);

    const std::vector<Value*>& elements() const;

private:
    std::vector<Value*> m_elements;
};

json::Document value_to_bdoc(const Value &value);

Value* create_integer(const int32_t value);
Value* create_document(const json::Document &doc);
Value* create_string(const std::string &str);
Value* create_tuple(Value *first, Value *second);
Value* create_float(const double &f);
Value* create_boolean(const bool value);
Value* create_list(const std::vector<std::string> &list);
Value* create_none();

}
