#include <memory>
#include <map>

#include "chipy/Interpreter.h"
#include <glog/logging.h>

#include "modules/modules.h"

namespace chipy
{

enum class CompareOpType
{
    Undefined,
    Equals,
    In,
    Is,
    IsNot,
    Less,
    LessEqual,
    More,
    MoreEqual,
    NotEqual,
    NotIn,
};

enum class BoolOpType
{
    Undefined,
    And,
    Or
};

enum class BinaryOpType
{
    Undefined,
    Add,
    BitAnd,
    BitOr,
    BitXor,
    Div,
    FloorDiv,
    LeftShift,
    Mod,
    Mult,
    Power,
    RightShift,
    Sub
};

enum class UnaryOpType
{
    Undefined,
    Add,
    Invert,
    Not,
    Sub,
};

class CppAttribute : public Callable
{
public:
    CppAttribute(CppObject &obj, const std::string& name)
        : m_obj(obj), m_name(name)
    {
    }

    ValueType type() const override
    {
        return ValueType::Attribute;
    }

    Value* call(const std::vector<Value*>& args) override
    {
        return m_obj.call_function(m_name, args);
    }

    Value* duplicate() const
    {
        return new CppAttribute(m_obj, m_name);
    }

private:
    CppObject &m_obj;
    const std::string m_name;
};

enum class BuiltinType
{
    Range,
    MakeInt,
    MakeString,
    Print
};

class RangeIterator : public Generator
{
public:
    RangeIterator(int32_t start, int32_t end, int32_t step_size)
        : m_start(start), m_end(end), m_step_size(step_size)
    {
        m_pos = m_start;
    }

    Value* duplicate() const override
    {
        return new RangeIterator(m_start, m_end, m_step_size);
    }

    Value* next() throw(stop_iteration_exception) override
    {
        if(m_pos >= m_end)
            throw stop_iteration_exception();

        auto res = new IntVal(m_pos);
        m_pos += m_step_size;

        return res;
    }

private:
    int32_t m_pos;
    const int32_t m_start, m_end, m_step_size;
};

class Builtin : public Callable
{
public:
    Builtin(BuiltinType type)
        : m_type(type)
    {}

    Value* duplicate() const
    {
        return new Builtin(m_type);
    }

    ValueType type() const
    {
        return ValueType::Builtin;
    }

    Value* call(const std::vector<Value*> &args)
    {
        if(m_type == BuiltinType::Range)
        {
            //TODO implemeent the rest...
            if(args.size() != 1)
                throw std::runtime_error("Invalid number of arguments");

            auto arg = args[0];

            if(arg == nullptr || arg->type() != ValueType::Integer)
                throw std::runtime_error("invalid argument type");

            return new RangeIterator(0, dynamic_cast<const IntVal*>(arg)->get(), 1);
        }
        else if(m_type == BuiltinType::MakeString)
        {
            if(args.size() != 1)
                throw std::runtime_error("Invalid number of arguments");

            auto arg = args[0];
            if(arg->type() == ValueType::String)
                return arg;
            else if(arg->type() == ValueType::Integer)
            {
                auto i = dynamic_cast<const IntVal*>(arg)->get();
                arg->drop();

                return create_string(std::to_string(i));
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
                return arg;
            else if(arg->type() == ValueType::String)
            {
                std::string s = dynamic_cast<StringVal*>(arg)->get();
                arg->drop();

                char *endptr = nullptr;
                return create_integer(strtol(s.c_str(), &endptr, 10));
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
            log_info("Program says: " + dynamic_cast<StringVal*>(arg)->get());
#else
            LOG(INFO) << "Program says: " << dynamic_cast<StringVal*>(arg)->get();
#endif
        }
        else
            throw std::runtime_error("Unknown builtin type");

        return nullptr;
    }

private:
    const BuiltinType m_type;
};

class CppObjectValue : public Iterator
{
public:
    CppObjectValue(CppObject *obj, bool owns_object)
        : m_obj(obj), m_owns_object(owns_object)
    {}

    virtual ~CppObjectValue()
    {
        if(m_owns_object)
            delete m_obj;
    }

    bool is_generator() const override
    {
        return true; // TODO actually check
    }

    Value* next() throw(stop_iteration_exception) override
    {
        return m_obj->call_function("__next__", {});
    }

    CppAttribute* get_attribute(const std::string &name)
    {
        return new CppAttribute(*m_obj, name);
    }

    Value* duplicate() const
    {
        if(m_owns_object)
            throw std::runtime_error("Can't duplicate cpp object");

        return new CppObjectValue(m_obj, false);
    }

    ValueType type() const override
    {
        return ValueType::CppObject;
    }

private:
    CppObject *m_obj;
    bool m_owns_object;
};

Value* create_tuple(Value *first, Value *second)
{
    return new Tuple(first, second);
}

Value* create_string(const std::string &str)
{
    return new StringVal(str);
}

Value* create_integer(const int32_t value)
{
    return new IntVal(value);
}

Value* create_boolean(const bool value)
{
    return new BoolVal(value);
}

Value* create_float(const double& value)
{
    return new FloatVal(value);
}

Value* create_none()
{
    return nullptr;
}

Value* wrap_cpp_object(CppObject *obj)
{
    return new CppObjectValue(obj, true);
}

Value* create_list(const std::vector<std::string> &list)
{
    auto val = new List();

    for(auto &e: list)
    {
        auto s = create_string(e);
        val->append(s);
        s->drop();
    }

    return val;
}

class Scope
{
public:
    const std::string BUILTIN_STR_NONE = "None";
    const std::string BUILTIN_STR_RANGE = "range";
    const std::string BUILTIN_STR_MAKE_INT = "int";
    const std::string BUILTIN_STR_MAKE_STR = "str";
    const std::string BUILTIN_STR_PRINT = "print";

    Scope() : m_parent(nullptr) {}
    Scope(Scope &parent) : m_parent(&parent) {}

    ~Scope()
    {
        for(auto it: m_values)
        {
            auto val = it.second;

            if(!val)
                continue;

            val->drop();
        }
    }

    void set_value(const std::string &id, Value* value)
    {
        if(m_parent && m_parent->has_value(id))
        {
            m_parent->set_value(id, value);
            return;
        }

        // FIXME actually update references...
        auto it = m_values.find(id);
        if(it != m_values.end())
        {
            if(it->second)
                it->second->drop();
            m_values.erase(it);
        }

        m_values[id] = value;

        if(value)
            value->raise();
    }

    bool has_value(const std::string &id)
    {
        if(m_parent && m_parent->has_value(id))
            return true;

        return m_values.find(id) != m_values.end();
    }

    Value* get_value(const std::string &id)
    {
        if(id == BUILTIN_STR_NONE)
            return nullptr;
        else if(id == BUILTIN_STR_RANGE)
            return new Builtin(BuiltinType::Range);
        else if(id == BUILTIN_STR_MAKE_INT)
            return new Builtin(BuiltinType::MakeInt);
        else if(id == BUILTIN_STR_MAKE_STR)
            return new Builtin(BuiltinType::MakeString);
        else if(id == BUILTIN_STR_PRINT)
            return new Builtin(BuiltinType::Print);

        auto it = m_values.find(id);
        if(it == m_values.end())
        {
            if(m_parent)
                return m_parent->get_value(id);
            else
                throw std::runtime_error("No such value: " + id);
        }

        auto val = it->second;
        if(val)
            val->raise();
        return val;
    }

    void terminate()
    {
        m_terminated = true;
    }

    bool is_terminated() const
    {
        return m_terminated;
    }

private:
    Scope *m_parent;
    bool m_terminated = false;
    std::map<std::string, Value*> m_values;
};

void Interpreter::load_from_module(Scope &scope, const std::string &mname, const std::string &name, const std::string &as_name)
{
    Module *module = nullptr;

    if(mname == "rand")
    {
        module = &g_module_rand;
    }
    else
        throw std::runtime_error("Unknown module: " + mname);

    scope.set_value(as_name == "" ? name: as_name, module->get_member(name));
}

void Interpreter::load_module(Scope &scope, const std::string &name, const std::string &as_name)
{
    Value *module = nullptr;

    if(name == "rand")
    {
        module = &g_module_rand;
    }
    else
        throw std::runtime_error("Unknown module: " + name);

    scope.set_value(as_name == "" ? name: as_name, module);
}

bool Interpreter::execute()
{
    LoopState loop_state = LoopState::None;
    Value *val = execute_next(*m_global_scope, loop_state);

    if(!val || val->type() != ValueType::Bool)
        throw std::runtime_error("result is not a boolean");

    bool res = dynamic_cast<BoolVal*>(val)->get();
    val->drop();
    return res;
}

std::vector<std::string> Interpreter::read_names()
{
    std::vector<std::string> result;

    NodeType type;
    m_data >> type;

    if(type == NodeType::Name)
    {
        std::string str;
        m_data >> str;
        result.push_back(str);
    }
    else if(type == NodeType::String)
    {
        std::string str;
        m_data >> str;
        result.push_back(str);
    }
    else if(type == NodeType::Tuple)
    {
        uint32_t num_elems = 0;
        m_data >> num_elems;

        if(num_elems != 2)
            throw std::runtime_error("Can only handle pairs");

        result.push_back(read_name());
        result.push_back(read_name());
    }
    else
        throw std::runtime_error("Not a valid name");

    return result;
}

std::string Interpreter::read_name()
{
    NodeType type;
    m_data >> type;

    if(type == NodeType::Name)
    {
        std::string str;
        m_data >> str;
        return str;
    }
    else if(type == NodeType::String)
    {
        std::string str;
        m_data >> str;
        return str;
    }
    else
        throw std::runtime_error("Not a valid name");
}

Value* Interpreter::execute_next(Scope &scope, LoopState &loop_state)
{
    auto start = m_data.pos();
    Value *returnval = nullptr;

    NodeType type;
    m_data >> type;

    LoopState dummy_loop_state = LoopState::None;

    switch(type)
    {
    case NodeType::ImportFrom:
    {
        auto module = read_name();
    
        auto val = execute_next(scope, dummy_loop_state);
        auto alias = value_cast<Alias>(val);
        load_from_module(scope, module, alias->name(), alias->as_name());
        alias->drop();
        break;
    }
    case NodeType::Import:
    {
        auto val = execute_next(scope, dummy_loop_state);
        auto alias = value_cast<Alias>(val);
        load_module(scope, alias->name(), alias->as_name());
        alias->drop();
        break;
    }
    case NodeType::Alias:
    {
        std::string name, as_name;
        m_data >> name >> as_name;
        returnval = new Alias(name, as_name);
        break;
    }
    case NodeType::Attribute:
    {
        Value *value = execute_next(scope, dummy_loop_state);
        std::string name = read_name();

        if(value->type() == ValueType::CppObject)
        {
            returnval = dynamic_cast<CppObjectValue*>(value)->get_attribute(name);
        }
        else if(value->type() == ValueType::Module)
        {
            returnval = dynamic_cast<Module*>(value)->get_member(name);
        }
        else if(value->type() == ValueType::Dictionary && name == "items")
        {
            returnval = dynamic_cast<Dictionary*>(value)->items();
        }
        else
            throw std::runtime_error("Cannot get attribute");

        break;
    }
    case NodeType::Name:
    {
        std::string str;
        m_data >> str;

        if(str == "False")
            returnval = new BoolVal(false);
        else if(str == "True")
            returnval = new BoolVal(true);
        else
            returnval = scope.get_value(str);
        break;
    }
    case NodeType::Continue:
    {
        if(loop_state == LoopState::None)
            throw std::runtime_error("Not a loop");

        loop_state = LoopState::Continue;
        break;
    }
    case NodeType::Break:
    {
        if(loop_state == LoopState::None)
            throw std::runtime_error("Not a loop");

        loop_state = LoopState::Break;
        break;
    }
    case NodeType::Assign:
    {
        auto val = execute_next(scope, dummy_loop_state);

        uint32_t num_targets = 0;
        m_data >> num_targets;

        for(uint32_t i = 0; i < num_targets; ++i)
        {
            auto names = read_names();

            if(names.size() == 1)
                scope.set_value(names[0], val);
            else if(names.size() == 2)
            {
                if(val->type() != ValueType::Tuple)
                    throw std::runtime_error("cannot unpack value");

                auto t = dynamic_cast<Tuple*>(val);

                scope.set_value(names[0], t->first());
                scope.set_value(names[1], t->second());
            }
            else
                throw std::runtime_error("invalid number of names");
        }

        if(val)
            val->drop();
        break;
    }
    case NodeType::StatementList:
    {
        uint32_t size = 0;
        m_data >> size;

        Value *final = nullptr;

        for(uint32_t i = 0; i < size; ++i)
        {
            if(scope.is_terminated() || loop_state == LoopState::Break || loop_state == LoopState::Continue)
                skip_next();
            else
            {
                if(final)
                    final->drop();

                LoopState child_loop_state = loop_state;
                if(child_loop_state == LoopState::TopLevel)
                    child_loop_state = LoopState::Normal;

                final = execute_next(scope, child_loop_state);

                if(loop_state != LoopState::None && child_loop_state != LoopState::Normal)
                    loop_state = child_loop_state;
            }
        }

        returnval = final;
        break;
    }
    case NodeType::UnaryOp:
    {
        UnaryOpType type;
        m_data >> type;

        auto res = execute_next(scope, dummy_loop_state);

        if(res && res->type() == ValueType::Bool)
        {
            bool cond = dynamic_cast<BoolVal*>(res)->get();
            res->drop();

            switch(type)
            {
            case UnaryOpType::Not:
                returnval = new BoolVal(!cond);
                break;
            default:
                throw std::runtime_error("Unknown unary operation");
            }
        }
        else if(res && res->type() == ValueType::Integer)
        {
            int32_t i = dynamic_cast<const IntVal*>(res)->get();
            res->drop();

            switch(type)
            {
            case UnaryOpType::Sub:
                returnval = new IntVal((-1)*i);
                break;
            case UnaryOpType::Not:
                returnval = new BoolVal(false);
                break;
            default:
                throw std::runtime_error("Unknown unary operation");
            }
        }
        else
        {
            switch(type)
            {
            case UnaryOpType::Not:
                returnval = new BoolVal(res == nullptr);

                if(res)
                    res->drop();
                break;
            default:
                throw std::runtime_error("Unknonw unary op");
            }
        }

        break;
    }
    case NodeType::BoolOp:
    {
        BoolOpType type;
        uint32_t num_vals = 0;
        m_data >> type >> num_vals;

        bool res;

        if(type == BoolOpType::And)
        {
            res = true;

            for(uint32_t i = 0; i < num_vals; ++i)
            {
                if(!res)
                {
                    skip_next();
                    continue;
                }

                auto val = execute_next(scope, dummy_loop_state);

                if(!val)
                {
                    res = false;
                    continue;
                }

                if(val->type() != ValueType::Bool)
                    throw std::runtime_error("not a valid bool operation");

                if(!dynamic_cast<const BoolVal*>(val)->get())
                    res = false;

                val->drop();
            }

        }
        else if(type == BoolOpType::Or)
        {
            res = false;

             for(uint32_t i = 0; i < num_vals; ++i)
             {
                 if(res)
                 {
                     skip_next();
                     continue;
                 }

                 auto val = execute_next(scope, dummy_loop_state);

                 if(!val)
                     continue;

                 if(val->type() != ValueType::Bool)
                     throw std::runtime_error("not a valid bool operation");

                 if(dynamic_cast<const BoolVal*>(val)->get())
                     res = true;

                 val->drop();
             }
        }
        else
            throw std::runtime_error("unknown bool op type");

        returnval = new BoolVal(res);
        break;
    }
    case NodeType::BinaryOp:
    {
        BinaryOpType type;
        m_data >> type;

        auto left = execute_next(scope, dummy_loop_state);
        auto right = execute_next(scope, dummy_loop_state);

        switch(type)
        {
        case BinaryOpType::Add:
        {
            if(left->type() == ValueType::Integer && right->type() == ValueType::Integer)
            {
                auto i1 = dynamic_cast<const IntVal*>(left)->get();
                auto i2 = dynamic_cast<const IntVal*>(right)->get();

                returnval = new IntVal(i1 + i2);
            }
            else if(left->type() == ValueType::String && right->type() == ValueType::String)
            {
                auto s1 = dynamic_cast<const StringVal*>(left)->get();
                auto s2 = dynamic_cast<const StringVal*>(right)->get();

                returnval = new StringVal(s1 + s2);
            }
            else
                throw std::runtime_error("failed to add");

            break;
        }
        case BinaryOpType::Sub:
        {
            if(!left || !right)
            {
                throw std::runtime_error("Cannot subtract on none values");
            }
            else if(left->type() == ValueType::Integer && right->type() == ValueType::Integer)
            {
                auto i1 = dynamic_cast<IntVal*>(left)->get();
                auto i2 = dynamic_cast<IntVal*>(right)->get();

                returnval = new IntVal(i1 - i2);
            }
            else
                throw std::runtime_error("failed to sub");
            break;
        }
        default:
            throw std::runtime_error("Unknown binary operation");
        }

        left->drop();
        right->drop();
        break;
    }
    case NodeType::Return:
    {
        returnval = execute_next(scope, dummy_loop_state);
        scope.terminate();
        break;
    }
    case NodeType::List:
    {
        auto list = new List();

        uint32_t size = 0;
        m_data >> size;

        for(uint32_t i = 0; i < size; ++i)
        {
            auto res = execute_next(scope, dummy_loop_state);
            list->append(res);
            res->drop();
        }

        returnval = list;
        break;
    }
    case NodeType::String:
    {
        std::string str;
        m_data >> str;

        returnval = new StringVal(str);
        break;
    }
    case NodeType::Compare:
    {
        auto current = execute_next(scope, dummy_loop_state);

        uint32_t size = 0;
        m_data >> size;

        for(uint32_t i = 0; i < size; ++i)
        {
            CompareOpType op_type;
            m_data >> op_type;

            Value *rval = execute_next(scope, dummy_loop_state);
            bool res = false;

            if(op_type == CompareOpType::Equals)
            {
                res = (*current == *rval);
            }
            else if(op_type == CompareOpType::MoreEqual)
            {
                res = (*current >= *rval);
            }
            else if(op_type == CompareOpType::More)
            {
                res = (*current > *rval);
            }
            else if(op_type == CompareOpType::In)
            {
                if(rval->type() != ValueType::List)
                    throw std::runtime_error("Can only call in on lists");

                res = dynamic_cast<const List*>(rval)->contains(*current);
            }
            else if(op_type == CompareOpType::NotEqual)
            {
                res = !(*current == *rval);
            }
            else if(op_type == CompareOpType::NotIn)
            {
                if(rval->type() != ValueType::List)
                    throw std::runtime_error("Can only call in on lists");

                res = !dynamic_cast<const List*>(rval)->contains(*current);
            }
            else if(op_type == CompareOpType::LessEqual)
            {
                res = (*rval >= *current);
            }
            else if(op_type == CompareOpType::Less)
            {
                res = (*rval > *current);
            }
            else
                throw std::runtime_error("Unknown op type");

            rval->drop();
            current->drop();
            current = new BoolVal(res);
        }

        returnval = current;
        break;
    }
    case NodeType::Index:
    {
        returnval = execute_next(scope, dummy_loop_state);
        break;
    }
    case NodeType::Integer:
    {
        int32_t val;
        m_data >> val;
        returnval = new IntVal(val);
        break;
    }
    case NodeType::Call:
    {
        auto callable = execute_next(scope, dummy_loop_state);
        if(!callable->is_callable())
            throw std::runtime_error("Cannot call un-callable!");

        uint32_t num_args = 0;
        m_data >> num_args;

        std::vector<Value*> args;
        for(uint32_t i = 0; i < num_args; ++i)
        {
            auto arg = execute_next(scope, dummy_loop_state);
            args.push_back(arg);
        }

        returnval = dynamic_cast<Callable*>(callable)->call(args);

        for(auto arg: args)
        {
            arg->drop();
        }

        callable->drop();
        break;
    }
    case NodeType::If:
    {
        auto test = execute_next(scope, loop_state);
        bool cond = test->bool_test();
        test->drop();

        Value *res = nullptr;

        if(cond)
            res = execute_next(scope, loop_state);
        else
            skip_next();

        returnval = res;
        break;
    }
    case NodeType::IfElse:
    {
        auto test = execute_next(scope, loop_state);
        if(test->type() != ValueType::Bool)
            throw std::runtime_error("not a boolean!");

        bool cond = dynamic_cast<BoolVal*>(test)->get();
        test->drop();

        if(cond)
        {
            returnval = execute_next(scope, loop_state);
            skip_next();
        }
        else
        {
            skip_next();
            returnval = execute_next(scope, loop_state);
        }

        break;
    }
    case NodeType::Dictionary:
    {
        auto res = new Dictionary();

        uint32_t size;
        m_data >> size;

        for(uint32_t i = 0; i < size; ++i)
        {
            auto name = read_name();
            auto value = execute_next(scope, dummy_loop_state);

            res->insert(name, value);
            value->drop();
        }

        returnval = res;
        break;
    }
    case NodeType::Subscript:
    {
        auto slice = execute_next(scope, dummy_loop_state);
        auto val = execute_next(scope, dummy_loop_state);

        if(val->type() == ValueType::Dictionary && slice->type() == ValueType::String)
        {
            returnval = dynamic_cast<Dictionary*>(val)->get(dynamic_cast<StringVal*>(slice)->get());
        }
        else if(val->type() == ValueType::List && slice->type() == ValueType::Integer)
        {
             returnval = dynamic_cast<List*>(val)->get(dynamic_cast<IntVal*>(slice)->get());
        }
        else
            throw std::runtime_error("Invalid subscript");

        returnval->raise();

        slice->drop();
        val->drop();

        break;
    }
    case NodeType::WhileLoop:
    {
        LoopState for_loop_state = LoopState::TopLevel;
        auto start = m_data.pos();
 
        while(for_loop_state != LoopState::Break)
        {
            m_data.move_to(start);

            auto test = execute_next(scope, dummy_loop_state);
            bool cond = test && test->bool_test();
            
            if(test)
                test->drop();

            if(!cond)
            {
                skip_next();
                break;
            }

            Scope body_scope(scope);
            execute_next(body_scope, for_loop_state);
        }
        break;
    }
    case NodeType::ForLoop:
    {
        std::vector<std::string> names = read_names();
        LoopState for_loop_state = LoopState::TopLevel;

        auto obj = execute_next(scope, dummy_loop_state);
        Iterator *iter = nullptr;

        if(obj->is_generator())
            iter = dynamic_cast<Iterator*>(obj);
        else if(obj->can_iterate())
        {
            iter = dynamic_cast<IterateableValue*>(obj)->iterate();
            obj->drop();
        }

        if(!iter)
            throw std::runtime_error("Can't iterate");

        while(for_loop_state != LoopState::Break)
        {
            Scope body_scope(scope);
            Value *next = nullptr;

            try {
                next = iter->next();
            } catch(stop_iteration_exception) {
                break;
            }

            if(names.size() == 1)
            {
                body_scope.set_value(names[0], next);
            }
            else if(names.size() == 2)
            {
                auto t = dynamic_cast<Tuple*>(next);
                if(!t)
                    throw std::runtime_error("Not a tuple!");

                body_scope.set_value(names[0], t->first());
                body_scope.set_value(names[1], t->second());
            }
            else
                throw std::runtime_error("Cannot handle more than two names");

            next->drop();
            execute_next(body_scope, for_loop_state);
        }

        skip_next();
        iter->drop();
        break;
    }
    case NodeType::AugmentedAssign:
    {
        BinaryOpType op_type;
        m_data >> op_type;

        auto t_name = read_name();
        auto target = scope.get_value(t_name);

        auto value = execute_next(scope, dummy_loop_state);

        switch(op_type)
        {
        case BinaryOpType::Add:
        {
            if(!target || !value || target->type() != ValueType::Integer || value->type() != ValueType::Integer)
                throw std::runtime_error("Values need to be numerics");

            auto i_target = dynamic_cast<IntVal*>(target);
            auto i_value  = dynamic_cast<const IntVal*>(value);
            i_target->set(i_target->get() + i_value->get());
            break;
        }
        default:
            throw std::runtime_error("Unknown binary op");
        }

        value->drop();
        target->drop();
        break;
    }
    default:
        throw std::runtime_error("Unknown node type!");
    }

    if(loop_state != LoopState::Normal && loop_state != LoopState::None)
        m_data.move_to(start);

    return returnval;
}

void Interpreter::skip_next()
{
    NodeType type;
    m_data >> type;

    switch(type)
    {
    case NodeType::Name:
    {
        std::string str;
        m_data >> str;
        break;
    }
    case NodeType::ForLoop:
    {
        read_names();
        skip_next();
        skip_next();
        break;
    }
    case NodeType::Assign:
    {
        skip_next();

        uint32_t num_targets = 0;
        m_data >> num_targets;

        for(uint32_t i = 0; i < num_targets; ++i)
        {
            skip_next();
        }
        break;
    }
    case NodeType::StatementList:
    {
        uint32_t size = 0;
        m_data >> size;

        for(uint32_t i = 0; i < size; ++i)
        {
            skip_next();
        }
        break;
    }
    case NodeType::Index:
    case NodeType::Return:
    {
        skip_next();
        break;
    }
    case NodeType::String:
    {
        std::string str;
        m_data >> str;
        break;
    }
    case NodeType::Compare:
    {
        skip_next();
        uint32_t size = 0;
        m_data >> size;

        for(uint32_t i = 0; i < size; ++i)
        {
            CompareOpType op_type;
            m_data >> op_type;
            skip_next();
        }
        break;
    }
    case NodeType::Integer:
    {
        int32_t val;
        m_data >> val;
        break;
    }
    case NodeType::Call:
    {
        skip_next();
        uint32_t num_args = 0;
        m_data >> num_args;
        for(uint32_t i = 0; i < num_args; ++i)
            skip_next();
        break;
    }
    case NodeType::IfElse:
    {
        skip_next();
        skip_next();
        skip_next();
        break;
    }
    case NodeType::List:
    case NodeType::Tuple:
    {
        uint32_t size;
        m_data >> size;
        for(uint32_t i = 0; i < size; ++i)
            skip_next();
        break;
    }
    case NodeType::Dictionary:
    {
        uint32_t size;
        m_data >> size;
        for(uint32_t i = 0; i < size; ++i)
        {
            skip_next();
            skip_next();
        }
        break;
    }
    case NodeType::UnaryOp:
    {
        UnaryOpType type;
        m_data >> type;
        skip_next();
        break;
    }
    case NodeType::BoolOp:
    {
        BoolOpType op;
        uint32_t num_vals = 0;
        m_data >> op >> num_vals;

        for(uint32_t i = 0l; i < num_vals; ++i)
            skip_next();
        break;
    }
    case NodeType::AugmentedAssign:
    case NodeType::BinaryOp:
    {
        BinaryOpType op;
        m_data >> op;
        skip_next();
        skip_next();
        break;
    }
    case NodeType::If:
    case NodeType::Attribute:
    case NodeType::Subscript:
    {
        skip_next();
        skip_next();
        break;
    }
    case NodeType::Break:
    case NodeType::Continue:
        break;
    default:
        throw std::runtime_error("Failed to skip unknown node type!");
    }
}

void Interpreter::set_object(const std::string &name, CppObject &obj)
{
    auto val = new CppObjectValue(&obj, false);
    m_global_scope->set_value(name, val);
    val->drop();
}

void Interpreter::set_string(const std::string &name, const std::string &value)
{
    auto s = create_string(value);
    m_global_scope->set_value(name, s);
    s->drop();
}

void Interpreter::set_list(const std::string &name, const std::vector<std::string> &list)
{
    auto l = create_list(list);
    m_global_scope->set_value(name, l);
    l->drop();
}

Interpreter::Interpreter(const BitStream &data)
{
    m_global_scope = new Scope();
    m_data.assign(data.data(), data.size(), true);
}

Interpreter::~Interpreter()
{
    delete m_global_scope;
}

}


