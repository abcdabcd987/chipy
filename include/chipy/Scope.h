#pragma once

#include "List.h"
#include "Tuple.h"
#include "Value.h"
#include "Dictionary.h"

namespace chipy
{

class Scope : public Object
{
public:
    const std::string BUILTIN_STR_NONE = "None";
    const std::string BUILTIN_STR_RANGE = "range";
    const std::string BUILTIN_STR_MAKE_INT = "int";
    const std::string BUILTIN_STR_MAKE_STR = "str";
    const std::string BUILTIN_STR_PRINT = "print";

    Scope(MemoryManager &mem) : Object(mem), m_parent(nullptr) {}
    Scope(MemoryManager &mem, Scope &parent) : Object(mem), m_parent(&parent) {}

    ~Scope();

    ValuePtr get_value(const std::string &id);
    void set_value(const std::string &name, Value* value);
    bool has_value(const std::string &name) const;
    void terminate();
    bool is_terminated() const;

    IntVal* create_integer(const int32_t value);
    ValuePtr create_document(const json::Document &doc);
    Dictionary* create_dictionary();
    StringVal* create_string(const std::string &str);
    Tuple* create_tuple(Value *first, Value *second);
    FloatVal* create_float(const double &f);
    BoolVal* create_boolean(const bool value);
    List* create_list();
    Value* create_none();

private:
    Scope *m_parent;
    bool m_terminated = false;

    std::unordered_map<std::string, Value*> m_values;
};

}
