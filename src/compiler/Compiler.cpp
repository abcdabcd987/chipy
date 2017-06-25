#include "json/BitStream.h"
#include "chipy/NodeType.h"

#include "pypa/reader.hh"
#include "pypa/filebuf.hh"
#include "pypa/lexer/lexer.hh"
#include "pypa/ast/ast.hh"
#include "pypa/parser/parser.hh"

namespace chipy
{

class StringReader : public pypa::Reader
{
public:
    StringReader(const std::string &code)
        : m_line_pos(0)
    {
        size_t last_pos = 0;
        size_t pos = std::string::npos;

        while((pos = code.find_first_of('\n', last_pos)) != std::string::npos)
        {
            m_code.push_back(code.substr(last_pos, pos+1 - last_pos));
            last_pos = pos+1;
        }

        m_code.push_back(code.substr(last_pos, code.size()-last_pos));
    }

    uint32_t get_line_number() const override
    {
        return m_line_pos;
    }

    std::string get_line() override
    {
        if(eof())
            return "";

        auto res = m_code[m_line_pos];
        m_line_pos += 1;
        return res;
    }

    std::string get_filename() const override
    {
        return "code";
    }

    bool set_encoding(const std::string &coding)
    {
        return true;
    }

    bool eof() const override
    {
        return m_line_pos == m_code.size();
    }

private:
    uint32_t m_line_pos;
    std::vector<std::string> m_code;
};

class Compiler
{
public:
    Compiler(const pypa::AstModulePtr ast)
        : m_ast(ast)
    {}

    void run()
    {
        parse_next(*(m_ast->body));
    }

    BitStream get_result()
    {
        uint8_t *data = nullptr;
        uint32_t len = 0;
        m_result.detach(data, len);

        BitStream res;
        res.assign(data, len, false);
        return res;
    }

private:
    void parse_next(const pypa::AstExpr& expr)
    {
        parse_next(reinterpret_cast<const pypa::Ast&>(expr));
    }

    void parse_expr_list(const pypa::AstExprList& list)
    {
        uint32_t size = list.size();
        m_result << size;

        for(auto item: list)
        {
            parse_next(*item);
        }
    }

    void parse_stmt_list(const pypa::AstStmtList& list)
    {
        uint32_t size = list.size();
        m_result << size;

        for(auto item: list)
        {
            parse_next(*item);
        }
    }

    void parse_next(const pypa::Ast &stmt)
    {
        switch(stmt.type)
        {
        case pypa::AstType::ImportFrom:
        {
            auto &import = reinterpret_cast<const pypa::AstImportFrom&>(stmt);
            m_result << NodeType::ImportFrom;

            parse_next(*import.module);
            parse_next(*import.names);
            break;
        }
        case pypa::AstType::Import:
        {
            auto &import = reinterpret_cast<const pypa::AstImport&>(stmt);
            m_result << NodeType::Import;
            parse_next(*import.names);
            break;
        }
        case pypa::AstType::Alias:
        {
            auto &alias = reinterpret_cast<const pypa::AstAlias&>(stmt);
            m_result << NodeType::Alias;
            const std::string name = reinterpret_cast<const pypa::AstName&>(*alias.name).id.c_str();

            if(alias.as_name)
            {
                const std::string as_name = reinterpret_cast<const pypa::AstName&>(*alias.as_name).id.c_str();
                m_result << name << as_name;
            }
            else
                m_result << name << std::string("");
            break;
        }
        case pypa::AstType::Name:
        {
            auto &exp = reinterpret_cast<const pypa::AstName&>(stmt);
            m_result << NodeType::Name;

            std::string str = exp.id.c_str();
            m_result << str;
            break;
        }
        case pypa::AstType::Assign:
        {
            auto &assign = reinterpret_cast<const pypa::AstAssign&>(stmt);
            m_result << NodeType::Assign;
            parse_next(*assign.value);
            parse_expr_list(assign.targets);
            break;
        }
        case pypa::AstType::Suite:
        {
            auto &suite = reinterpret_cast<const pypa::AstSuite&>(stmt);
            m_result << NodeType::StatementList;
            parse_stmt_list(suite.items);
            break;
        }
        case pypa::AstType::Str:
        {
            auto &str = reinterpret_cast<const pypa::AstStr&>(stmt);
            m_result << NodeType::String;
            m_result << std::string(str.value.c_str());
            break;
        }
        case pypa::AstType::Return:
        {
            auto &ret = reinterpret_cast<const pypa::AstReturn&>(stmt);
            m_result << NodeType::Return;
            parse_next(*ret.value);
            break;
        }
        case pypa::AstType::Dict:
        {
            auto &dict = reinterpret_cast<const pypa::AstDict&>(stmt);

            uint32_t size = dict.keys.size();
            m_result << NodeType::Dictionary << size;
            for(uint32_t i = 0; i < size; ++i)
            {
                parse_next(*dict.keys[i]);
                parse_next(*dict.values[i]);
            }
            break;
        }
        case pypa::AstType::Compare:
        {
            auto &comp = reinterpret_cast<const pypa::AstCompare&>(stmt);
            m_result << NodeType::Compare;
            parse_next(*comp.left);

            uint32_t size = comp.comparators.size();
            m_result << size;
            for(uint32_t i = 0; i < size; ++i)
            {
                m_result << comp.operators[i];
                parse_next(*comp.comparators[i]);
            }

            break;
        }
        case pypa::AstType::Number:
        {
            auto &num = reinterpret_cast<const pypa::AstNumber&>(stmt);

            if(num.num_type == pypa::AstNumber::Integer)
            {
                m_result << NodeType::Integer;
                int32_t i = num.integer;
                m_result << i;
            }
            else
                throw std::runtime_error("Unknown number type!");
            break;
        }
        case pypa::AstType::If:
        {
            auto &ifclause = reinterpret_cast<const pypa::AstIf&>(stmt);
            if(ifclause.orelse)
            {
                m_result << NodeType::IfElse;
                parse_next(*ifclause.test);
                parse_next(*ifclause.body);
                parse_next(*ifclause.orelse);
            }
            else
            {
                m_result << NodeType::If;
                parse_next(*ifclause.test);
                parse_next(*ifclause.body);
            }
            break;
        }
        case pypa::AstType::Call:
        {
            auto &call = reinterpret_cast<const pypa::AstCall&>(stmt);
            m_result << NodeType::Call;
            parse_next(*call.function);
            parse_expr_list(call.arglist.arguments);
            break;
        }
        case pypa::AstType::Attribute:
        {
            auto &attr = reinterpret_cast<const pypa::AstAttribute&>(stmt);
            m_result << NodeType::Attribute;
            parse_next(*attr.value);
            parse_next(*attr.attribute);
            break;
        }
        case pypa::AstType::UnaryOp:
        {
            auto &op = reinterpret_cast<const pypa::AstUnaryOp&>(stmt);
            m_result << NodeType::UnaryOp;
            m_result << op.op;
            parse_next(*op.operand);
            break;
        }
        case pypa::AstType::BoolOp:
       {
            auto &op = reinterpret_cast<const pypa::AstBoolOp&>(stmt);
            m_result << NodeType::BoolOp;
            m_result << op.op;
            m_result << static_cast<uint32_t>(op.values.size());

            for(auto v : op.values)
                parse_next(*v);
            break;
        }
        case pypa::AstType::BinOp:
        {
            auto &op = reinterpret_cast<const pypa::AstBinOp&>(stmt);
            m_result << NodeType::BinaryOp;
            m_result << op.op;
            parse_next(*op.left);
            parse_next(*op.right);

            break;
        }
        case pypa::AstType::List:
        {
            auto &list = reinterpret_cast<const pypa::AstList&>(stmt);
            m_result << NodeType::List;
            parse_expr_list(list.elements);
            break;
        }
        case pypa::AstType::Subscript:
        {
            auto &subs = reinterpret_cast<const pypa::AstSubscript&>(stmt);
            m_result << NodeType::Subscript;
            parse_next(*subs.slice);
            parse_next(*subs.value);
            break;
        }
        case pypa::AstType::Index:
        {
            auto &idx = reinterpret_cast<const pypa::AstIndex&>(stmt);
            m_result << NodeType::Index;
            parse_next(*idx.value);
            break;
        }
        case pypa::AstType::For:
        {
            auto &loop = reinterpret_cast<const pypa::AstFor&>(stmt);
            m_result << NodeType::ForLoop;
            parse_next(*loop.target);
            parse_next(*loop.iter);
            parse_next(*loop.body);
            break;
        }
        case pypa::AstType::While:
        {
            auto &loop = reinterpret_cast<const pypa::AstWhile&>(stmt);
            m_result << NodeType::WhileLoop;
            parse_next(*loop.test);
            parse_next(*loop.body);
            break;
        }
        case pypa::AstType::AugAssign:
        {
            auto &ass = reinterpret_cast<const pypa::AstAugAssign&>(stmt);
            m_result << NodeType::AugmentedAssign;
            m_result << ass.op;
            parse_next(*ass.target);
            parse_next(*ass.value);
            break;
        }
        case pypa::AstType::ExpressionStatement:
        {
            auto &expr = reinterpret_cast<const pypa::AstExpressionStatement&>(stmt);
            parse_next(*expr.expr);
            break;
        }
        case pypa::AstType::Continue:
        {
            m_result << NodeType::Continue;
            break;
        }
        case pypa::AstType::Break:
        {
            m_result << NodeType::Break;
            break;
        }
        case pypa::AstType::Tuple:
        {
            auto &t = reinterpret_cast<const pypa::AstTuple&>(stmt);
            m_result << NodeType::Tuple;
            m_result << static_cast<uint32_t>(t.elements.size());

            for(auto e: t.elements)
            {
                parse_next(*e);
            }
            break;
        }
       default:
            throw std::runtime_error("Unknown statement type!");
        }
    }

    const pypa::AstModulePtr m_ast;

    BitStream m_result;
};

BitStream compile_file(const std::string &filename)
{
    pypa::AstModulePtr ast;
    pypa::SymbolTablePtr symbols;
    pypa::ParserOptions options;
    options.python3only = true;
    options.printerrors = true;
    options.printdbgerrors = true;

    pypa::Lexer lexer(std::unique_ptr<pypa::Reader>{new pypa::FileBufReader(filename)});

    if(!pypa::parse(lexer, ast, symbols, options))
    {
        throw std::runtime_error("Parsing failed");
    }

    Compiler compiler(ast);
    compiler.run();

    return compiler.get_result();
}
BitStream compile_code(const std::string &code)
{
    pypa::AstModulePtr ast;
    pypa::SymbolTablePtr symbols;
    pypa::ParserOptions options;
    options.python3only = true;
    options.printerrors = true;
    options.printdbgerrors = true;

    pypa::Lexer lexer(std::unique_ptr<pypa::Reader>{new StringReader(code)});

    if(!pypa::parse(lexer, ast, symbols, options))
    {
        throw std::runtime_error("Parsing failed");
    }

    Compiler compiler(ast);
    compiler.run();

    return compiler.get_result();
}

}
