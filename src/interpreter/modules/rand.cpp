#include "modules.h"

namespace chipy
{

Value* RandModule::get_member(const std::string &name) const
{
    if(name == "randint")
        return new Function([](const std::vector<Value*> &args) -> Value*{
                if(args.size() != 2)
                    throw std::runtime_error("Invalid number of arguments");

                auto start = value_cast<IntVal>(args[0]);
                auto end = value_cast<IntVal>(args[1]);

                auto range = end->get() - start->get();
                
                return create_integer((rand() % range) + start->get());
             });
    else
        throw std::runtime_error("No such member: " + name);
}

RandModule g_module_rand;

}
