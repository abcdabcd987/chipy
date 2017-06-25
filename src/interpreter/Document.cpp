#include "chipy/Value.h"

namespace chipy
{

class DocConverter : public json::Iterator
{
public:
    Value* get_result()
    {
        // Not a valid document?
        if(parse_stack.size() != 1)
            throw std::runtime_error("Json converter in an invalid child");

        return parse_stack.top();
    }

    void handle_datetime(const std::string &key, const tm &value) override
    {
        (void)key;
        (void)value;
        throw std::runtime_error("Datetime not supported");
    }

    void handle_string(const std::string &key, const std::string &str)
    {
        auto val = create_string(str);
        add_value(key, val);
    }

    void handle_integer(const std::string &key, int64_t i) override
    {
        auto val = create_integer(i);
        add_value(key, val);
    }

    void handle_float(const std::string &key, const double value) override
    {
        auto val = create_float(value);
        add_value(key, val);
    }

    void handle_boolean(const std::string &key, const bool value) override
    {
        auto val = create_boolean(value);
        add_value(key, val);
    }

    void add_value(const std::string& key, Value *value)
    {
        if(key == "")
            parse_stack.push(value);
        else
            append_child(key, value);
    }

    void handle_null(const std::string &key) override
    {
        auto val = create_none();
        add_value(key, val);
    }

    void handle_map_start(const std::string &key) override
    {
        auto dict = new Dictionary();

        if(key != "")
            append_child(key, dict);
        parse_stack.push(dict);
    }

    void handle_map_end() override
    {
        if(parse_stack.size() > 1)
        {
            parse_stack.pop();
        }
    }

    void handle_array_start(const std::string &key) override
    {
        auto list = new List();
        if(key != "")
            append_child(key, list);
        parse_stack.push(list);
    }

    void handle_array_end() override
    {
        if(parse_stack.size() > 1)
        {
            parse_stack.pop();
        }
    }

    void handle_binary(const std::string &key, const uint8_t *data, uint32_t len) override
    {
        //FIXME
    }

private:
    void append_child(const std::string key, Value *obj)
    {
        if(parse_stack.size() == 0)
            throw std::runtime_error("cannot append child at this point!");

        auto &top = parse_stack.top();

        switch(top->type())
        {
        case ValueType::Dictionary:
        {
            auto &d = dynamic_cast<Dictionary&>(*top);
            d.insert(key, obj);
            break;
        }
        case ValueType::List:
        {
            auto &l = dynamic_cast<List&>(*top);
            l.append(obj);
            break;
        }
        default:
            throw std::runtime_error("Failed to append child");
        }
    }

    std::stack<Value*> parse_stack;
};

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

Value* create_document(const json::Document &doc)
{
    DocConverter converter;
    doc.iterate(converter);
    return converter.get_result();
}


}
