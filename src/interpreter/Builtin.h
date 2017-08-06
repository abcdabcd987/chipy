#pragma once

#include <glog/logging.h>
#include "RangeIterator.h"

namespace chipy
{

enum class BuiltinType
{
    Range,
    MakeInt,
    MakeString,
    Print
};

class Builtin : public Callable
{
public:
    Builtin(MemoryManager &mem, BuiltinType type)
        : Callable(mem), m_type(type)
    {}

    ValuePtr duplicate() override
    {
        return ValuePtr(new (memory_manager()) Builtin(memory_manager(), m_type));
    }

    ValueType type() const override
    {
        return ValueType::Builtin;
    }

    ValuePtr call(const std::vector<ValuePtr> &args) override
    {
        if(m_type == BuiltinType::Range)
        {
            //TODO implemeent the rest...
            if(args.size() != 1)
                throw std::runtime_error("Invalid number of arguments");

            auto arg = args[0];

            if(arg == nullptr || arg->type() != ValueType::Integer)
                throw std::runtime_error("invalid argument type");

            return ValuePtr(new (memory_manager()) RangeIterator(memory_manager(), 0, value_cast<IntVal>(arg)->get(), 1));
        }
        else if(m_type == BuiltinType::MakeString)
        {
            if(args.size() != 1)
                throw std::runtime_error("Invalid number of arguments");

            auto arg = args[0];
            if(arg->type() == ValueType::String)
            {
                return arg;
            }
            else if(arg->type() == ValueType::Integer)
            {
                auto i = value_cast<IntVal>(arg)->get();

                return wrap_value(new (memory_manager()) StringVal(memory_manager(), std::to_string(i)));
            }
            else
                throw std::runtime_error("Can't conver to integer");
        }

        else if(m_type == BuiltinType::MakeInt)
        {
            if(args.size() != 1)
                throw std::runtime_error("Invalid number of arguments");

            auto arg = args[0];
            if(arg->type() == ValueType::Integer)
            {
                return arg;
            }
            else if(arg->type() == ValueType::String)
            {
                std::string s = value_cast<StringVal>(arg)->get();
                char *endptr = nullptr;
                return wrap_value(new (memory_manager()) IntVal(memory_manager(), strtol(s.c_str(), &endptr, 10)));
            }
            else
                throw std::runtime_error("Can't conver to integer");
        }
        else if(m_type == BuiltinType::Print)
        {
            if(args.size() != 1)
                throw std::runtime_error("Invalid number of arguments");

            auto arg = args[0];

            if(arg->type() != ValueType::String)
                throw std::runtime_error("Argument not a string");

#ifdef IS_ENCLAVE
            log_info("Program says: " + value_cast<StringVal>(arg)->get());
#else
            LOG(INFO) << "Program says: " << value_cast<StringVal>(arg)->get();
#endif
        }
        else
            throw std::runtime_error("Unknown builtin type");

        return nullptr;
    }

private:
    const BuiltinType m_type;
};

}
