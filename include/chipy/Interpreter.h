#pragma once

#include <string>

#include "chipy/NodeType.h"

#include "Module.h"
#include "Value.h"
#include "Tuple.h"
#include "Scope.h"

namespace chipy
{

class Interpreter
{
public:
    Interpreter(const BitStream &data);
    ~Interpreter();

    bool execute();

    void set_module(const std::string& name, ModulePtr module);

    void set_list(const std::string& name, const std::vector<std::string> &list);
    void set_string(const std::string& name, const std::string &value);
    
    void set_module(const std::string& name, ModulePtr &module);

    MemoryManager& memory_manager()
    {
        return m_mem;
    }

private:
    enum class LoopState { None, TopLevel, Normal, Break, Continue };

    ModulePtr get_module(const std::string &name);

    ValuePtr execute_next(Scope &scope, LoopState &loop_state);
    void skip_next();

    void load_from_module(Scope &scope, const std::string &module, const std::string &name, const std::string &as_name);
    void load_module(Scope &scope, const std::string &name, const std::string &as_name);

    std::string read_name();
    std::vector<std::string> read_names();

    BitStream m_data;

    MemoryManager m_mem;
    Scope *m_global_scope;

    std::unordered_map<std::string, ModulePtr> m_loaded_modules;
};

}
