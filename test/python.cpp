#include "chipy/chipy.h"
#include <gtest/gtest.h>

using namespace chipy;

class PythonTest : public testing::Test
{
};

class FooObj : public CppObject
{
public:
    Value* call_function(const std::string& funcname, const std::vector<CppArg*> &arguments) override
    {
        return create_integer(42);
    }
};

class Foo2Obj : public CppObject
{
public:
    Value* call_function(const std::string& funcname, const std::vector<CppArg*> &args) override
    {
        if(args.size() == 0 || args[0]->Type != CppArg::Integer)
        {
            throw std::runtime_error("invalid argument(s)");
        }

        return create_integer(2 * args[0]->IntVal);
    }
};

TEST(PythonTest, greater_than)
{
    const std::string code = "i = 0\n"
                             "return i > -1";

    auto doc = compile_code(code);

    Interpreter pyint(doc);

    EXPECT_TRUE(pyint.execute());
}

TEST(PythonTest, call_cpp_with_argument)
{
    const std::string code =
            "f = foo.double(21)\n"
            "if f == 42:\n"
            "	return True\n"
            "else:\n"
            "	return False";

    auto doc = compile_code(code);

    Interpreter pyint(doc);
    Foo2Obj foo;
    pyint.set_object("foo", foo);

    auto res = pyint.execute();
    EXPECT_EQ(res, true);
}
TEST(PythonTest, call_cpp)
{
    const std::string code =
            "f = foo.get()\n"
            "if f == 42:\n"
            "	return True\n"
            "else:\n"
            "	return False";

    auto doc = compile_code(code);

    Interpreter pyint(doc);
    FooObj foo;
    pyint.set_object("foo", foo);

    auto res = pyint.execute();
    EXPECT_EQ(res, true);
}

TEST(PythonTest, array)
{
    const std::string code =
            "arr = [5,4,1337,2]\n"
            "if arr[2] == 1337:\n"
            "	return True\n"
            "return False";

    auto doc = compile_code(code);

    Interpreter pyint(doc);

    auto res = pyint.execute();

    EXPECT_EQ(res, true);
}

TEST(PythonTest, for_loop)
{
    const std::string code = "l = [1,2,3]\n"
                             "res = 0\n"
                             "for i in l:\n"
                             "   res += i\n"
                             "return res == 6";

    auto doc = compile_code(code);
    Interpreter pyint(doc);

    auto res = pyint.execute();

    EXPECT_TRUE(res);
}

TEST(PythonTest, dictionary)
{
    const std::string code =
            "i = {'value':42}\n"
            "if i['value'] == 42:\n"
            "	return True\n"
            "return False";

    auto doc = compile_code(code);

    Interpreter pyint(doc);

    auto res = pyint.execute();

    EXPECT_EQ(res, true);
}

TEST(PythonTest, if_clause)
{
    const std::string code =
            "i = 42\n"
            "if i == 43:\n"
            "	return False\n"
            "else:\n"
            "	return True";

    auto doc = compile_code(code);

    Interpreter pyint(doc);

    auto res = pyint.execute();

    EXPECT_EQ(res, true);
}

TEST(PythonTest, str_eq)
{
    const std::string code =
            "a = 'foo'\n"
            "b = 'foo'\n"
            "eq = (a == b)\n"
            "return eq";

    auto doc = compile_code(code);

    Interpreter pyint(doc);
    auto res = pyint.execute();

    EXPECT_EQ(res, true);
}

TEST(PythonTest, none_value)
{
    const std::string code =
            "a = None\n"
            "if not a:\n"
            "   return False\n"
            "return True";

    auto doc = compile_code(code);
    Interpreter pyint(doc);
    auto res = pyint.execute();

    EXPECT_FALSE(res);
}

TEST(PythonTest, logical_and)
{
    const std::string code =
            "a = False\n"
            "b = True\n"
            "return (not a) and b";

    auto doc = compile_code(code);
    Interpreter pyint(doc);
    auto res = pyint.execute();

    EXPECT_EQ(res, true);
}

TEST(PythonTest, create_by_assign)
{
    const std::string code =
            "a = 1\n"
            "b = a+1\n"
            "return b == 2";

    auto doc = compile_code(code);
    Interpreter pyint(doc);
    auto res = pyint.execute();

    EXPECT_EQ(res, true);
}

TEST(PythonTest, iterate_dict)
{
    const std::string code =
           "res  = 0\n"
           "dict = {'a':1, 'b':2}\n"
           "for t in dict.items():\n"
           "    k,v = t\n"
           "    if k == 'b':\n"
           "       res = v\n"
           "return res == 2";

    auto doc = compile_code(code);
    Interpreter pyint(doc);
    auto res = pyint.execute();

    EXPECT_EQ(res, true);
}

TEST(PythonTest, or_op)
{
    const std::string code =
           "dict = None\n"
           "if not dict or dict['b'] == 1:\n"
           "    return True\n"
           "else:\n"
           "    return False";

    auto doc = compile_code(code);
    Interpreter pyint(doc);
    auto res = pyint.execute();

    EXPECT_EQ(res, true);
}

TEST(PythonTest, iterate_dict2)
{
    const std::string code =
           "res  = 0\n"
           "dict = {'a':1, 'b':2}\n"
           "for k,v in dict.items():\n"
           "    if k == 'b':\n"
           "       res = v\n"
           "return res == 2";

    auto doc = compile_code(code);
    Interpreter pyint(doc);
    auto res = pyint.execute();

    EXPECT_EQ(res, true);
}

TEST(PythonTest, loop_break)
{
    const std::string code =
           "a = 5\n"
           "for _ in range(10):\n"
           "    a += 1\n"
           "    break\n"
           "return a == 6";

    auto doc = compile_code(code);
    Interpreter pyint(doc);
    auto res = pyint.execute();

    EXPECT_EQ(res, true);
}

TEST(PythonTest, loop_continue)
{
    const std::string code =
           "a = 5\n"
           "for _ in range(10):\n"
           "    continue\n"
           "    a += 1\n"
           "return a == 5";

    auto doc = compile_code(code);
    Interpreter pyint(doc);
    auto res = pyint.execute();

    EXPECT_EQ(res, true);
}


TEST(PythonTest, not)
{
    const std::string code =
            "b = False\n"
            "return not b";

    auto doc = compile_code(code);
    Interpreter pyint(doc);
    auto res = pyint.execute();

    EXPECT_EQ(res, true);
}

TEST(PythonTest, pre_set_list)
{
    const std::string code =
            "return b[1] == 'foo'";

    auto doc = compile_code(code);

    Interpreter pyint(doc);
    pyint.set_list("b", {"bar", "foo"});
    auto res = pyint.execute();

    EXPECT_EQ(res, true);
}

TEST(PythonTest, set_variable)
{
    const std::string code =
            "b = False \n"
            "return b";

    auto doc = compile_code(code);

    Interpreter pyint(doc);
    auto res = pyint.execute();

    EXPECT_EQ(res, false);
}

TEST(PythonTest, pre_set_value)
{
    std::string code = "if op_type == 'put':\n"
                      "    return False\n"
                      "else:\n"
                      "	   return True";

    auto data = compile_code(code);
    Interpreter interpreter(data);

    interpreter.set_string("op_type", "put");

    EXPECT_EQ(interpreter.execute(), false);
}
