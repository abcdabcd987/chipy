#pragma once

#include <string>

#include "chipy/NodeType.h"

#include "Value.h"

namespace chipy
{

class Scope;

class Interpreter
{
public:
    Interpreter(const BitStream &data);
    ~Interpreter();

    bool execute();

    void set_object(const std::string& name, CppObject &obj);

    void set_list(const std::string& name, const std::vector<std::string> &list);
    void set_string(const std::string& name, const std::string &value);

private:
    enum class LoopState { None, TopLevel, Normal, Break, Continue };

    Value* execute_next(Scope &scope, LoopState &loop_state);
    void skip_next();

    std::string read_name();
    std::vector<std::string> read_names();

    BitStream m_data;
    Scope *m_global_scope;
};
}
