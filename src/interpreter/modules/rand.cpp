#include "modules.h"

namespace chipy
{

ValuePtr RandModule::get_member(const std::string &name)
{
    if(name == "randint")
    {
        auto &mem = memory_manager();

        return wrap_value( new (mem) Function(mem,
              [&](const std::vector<ValuePtr> &args) -> ValuePtr {
                if(args.size() != 2)
                    throw std::runtime_error("Invalid number of arguments");

                auto start = value_cast<IntVal>(args[0]);
                auto end = value_cast<IntVal>(args[1]);

                auto range = end->get() - start->get();
                
                return wrap_value(new (memory_manager()) IntVal(mem, (rand() % range) + start->get()));
             }));
    }
    else
        throw std::runtime_error("No such member: " + name);
}

}
