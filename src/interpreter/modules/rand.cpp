#include "modules.h"
#include <random>

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

#ifdef IS_ENCLAVE
                //FIXME
                return wrap_value(new (memory_manager()) IntVal(mem, 0));
#else
                auto start = value_cast<IntVal>(args[0])->get();
                auto end = value_cast<IntVal>(args[1])->get();

                std::default_random_engine generator;
                std::uniform_int_distribution<int> distribution(start, end);
                
                return wrap_value(new (memory_manager()) IntVal(mem, distribution(generator)));
#endif
            }));
    }
    else
        throw std::runtime_error("No such member: " + name);
}

}
