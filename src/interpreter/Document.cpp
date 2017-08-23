#include "chipy/Value.h"
#include "chipy/Dictionary.h"
#include "chipy/List.h"

namespace chipy
{

void value_to_bdoc(const std::string &key, ValuePtr value, json::Writer &writer)
{
    switch(value->type())
    {
    case ValueType::Dictionary:
    {
        auto dict = value_cast<Dictionary>(value);
        writer.start_map(key);

        for(auto &e : dict->elements())
        {
            auto &key = e.first;
            auto &value = e.second;

            value_to_bdoc(key, value, writer);
        }

        writer.end_map();
        break;
    }
    case ValueType::List:
    {
        auto l = value_cast<List>(value);
        writer.start_array(key);

        for(auto &e : l->elements())
        {
            value_to_bdoc("", e, writer);
        }

        writer.end_map();
        break;
    }
    case ValueType::String:
    {
        auto str = value_cast<StringVal>(value);
        writer.write_string(key, str->get());
        break;
    }
    case ValueType::Integer:
    {
        auto i = value_cast<IntVal>(value);
        writer.write_integer(key, i->get());
        break;
    }
    default:
        throw std::runtime_error("Unknown value type");
    }
}

json::Document value_to_document(ValuePtr value)
{
    json::Writer writer;
    value_to_bdoc("", value, writer);

    return writer.make_document();
}

}
