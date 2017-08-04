#include "chipy/Value.h"
#include "chipy/Dictionary.h"

namespace chipy
{

void value_to_bdoc(const std::string &key, const Value &value, json::Writer &writer)
{
    switch(value.type())
    {
    case ValueType::Dictionary:
    {
        auto &dict = dynamic_cast<const Dictionary&>(value);
        writer.start_map(key);

        for(auto &e : dict.elements())
        {
            auto &key = e.first;
            auto &value = e.second;

            value_to_bdoc(key, *value, writer);
        }

        writer.end_map();
        break;
    }
    case ValueType::String:
    {
        auto &str = dynamic_cast<const StringVal&>(value);
        writer.write_string(key, str.get());
        break;
    }
    case ValueType::Integer:
    {
        auto &i = dynamic_cast<const IntVal&>(value);
        writer.write_integer(key, i.get());
        break;
    }
    default:
        throw std::runtime_error("Unknown value type");
    }
}

json::Document value_to_bdoc(const Value &value)
{
    json::Writer writer;
    value_to_bdoc("", value, writer);

    return writer.make_document();
}

}
