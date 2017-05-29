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
    Tuple
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

struct CppArg
{
    CppArg(int32_t i)
        : Type(Integer), IntVal(i)
    {}

    CppArg(const std::string& str)
        : Type(String), StrVal(str)
    {}

    CppArg(const std::vector<std::string> &list)
        : Type(List), ListVal(list)
    {}

    ~CppArg()
    {}

    const enum arg_t { Integer, String, List } Type;

    union
    {
        const std::string StrVal;
        const std::vector<std::string> ListVal;
        const int32_t IntVal;
    };
};


class CppObject
{
public:
    virtual ~CppObject() {}

    virtual Value* call_function(const std::string &func_name, const std::vector<CppArg*>& args) = 0;
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

Value* create_integer(const int32_t value);
Value* create_document(const json::Document &doc);
Value* create_string(const std::string &str);
Value* create_tuple(Value *first, Value *second);
Value* create_float(const double &f);
Value* create_boolean(const bool value);
Value* create_list(const std::vector<std::string> &list);
Value* create_none();

Value* wrap_cpp_object(CppObject *obj);

}
