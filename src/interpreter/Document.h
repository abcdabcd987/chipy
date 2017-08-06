#pragma once

namespace chipy
{

class DocConverter : public json::Iterator
{
public:
    DocConverter(Scope &scope)
        : m_scope(scope)
    {}

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
        auto val = m_scope.create_string(str);
        add_value(key, val);
    }

    void handle_integer(const std::string &key, int64_t i) override
    {
        auto val = m_scope.create_integer(i);
        add_value(key, val);
    }

    void handle_float(const std::string &key, const double value) override
    {
        auto val = m_scope.create_float(value);
        add_value(key, val);
    }

    void handle_boolean(const std::string &key, const bool value) override
    {
        auto val = m_scope.create_boolean(value);
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
        auto val = m_scope.create_none();
        add_value(key, val);
    }

    void handle_map_start(const std::string &key) override
    {
        auto dict = m_scope.create_dictionary();

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
        auto list = m_scope.create_list();
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

private:
    Scope &m_scope;
};

}