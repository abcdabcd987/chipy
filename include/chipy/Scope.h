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

    ValuePtr get_value(const std::string &id);
    void set_value(const std::string &name, ValuePtr value);
    bool has_value(const std::string &name) const;
    void terminate();
    bool is_terminated() const;

    IntValPtr create_integer(const int32_t value);
    ValuePtr create_document(const json::Document &doc);
    DictionaryPtr create_dictionary();
    StringValPtr create_string(const std::string &str);
    TuplePtr create_tuple(ValuePtr first, ValuePtr second);
    FloatValPtr create_float(const double &f);
    BoolValPtr create_boolean(const bool value);
    ListPtr create_list();
    ValuePtr create_none();

private:
    Scope *m_parent;
    bool m_terminated = false;

    std::unordered_map<std::string, ValuePtr> m_values;
};

}
