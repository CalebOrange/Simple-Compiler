#include "front/semantic.h"

#include <cassert>
#include <numeric>

using ir::Function;
using ir::Instruction;
using ir::Operand;
using ir::Operator;

#define TODO assert(0 && "TODO");

// 获取一个树节点的指定类型子节点的指针，并且在获取失败的情况下，使用 assert 断言来停止程序的执行
#define GET_CHILD_PTR(node, type, index)                     \
    auto node = dynamic_cast<type *>(root->children[index]); \
    assert(node);

// 获取一个树节点的指定类型子节点的指针，并调用一个名为 analysis<type> 的函数来对这个子节点进行分析
#define ANALYSIS(node, type, index)                          \
    auto node = dynamic_cast<type *>(root->children[index]); \
    assert(node);                                            \
    analysis##type(node, buffer);

// 将一个表达式节点的信息复制到另一个表达式节点中
#define COPY_EXP_NODE(from, to)              \
    to->is_computable = from->is_computable; \
    to->v = from->v;                         \
    to->t = from->t;

#define NODE_IS(node_type, index) root->children[index]->type == NodeType::node_type

map<std::string, ir::Function *> *frontend::get_lib_funcs()
{
    static map<std::string, ir::Function *> lib_funcs = {
        {"getint", new Function("getint", Type::Int)},
        {"getch", new Function("getch", Type::Int)},
        {"getfloat", new Function("getfloat", Type::Float)},
        {"getarray", new Function("getarray", {Operand("arr", Type::IntPtr)}, Type::Int)},
        {"getfarray", new Function("getfarray", {Operand("arr", Type::FloatPtr)}, Type::Int)},
        {"putint", new Function("putint", {Operand("i", Type::Int)}, Type::null)},
        {"putch", new Function("putch", {Operand("i", Type::Int)}, Type::null)},
        {"putfloat", new Function("putfloat", {Operand("f", Type::Float)}, Type::null)},
        {"putarray", new Function("putarray", {Operand("n", Type::Int), Operand("arr", Type::IntPtr)}, Type::null)},
        {"putfarray", new Function("putfarray", {Operand("n", Type::Int), Operand("arr", Type::FloatPtr)}, Type::null)},
    };
    return &lib_funcs;
}

void frontend::SymbolTable::add_scope()
{
    scope_stack.push_back(ScopeInfo());
}
void frontend::SymbolTable::exit_scope()
{
    scope_stack.pop_back();
}

string frontend::SymbolTable::get_scoped_name(string id) const
{
    return id + std::to_string(scope_stack.size());
}

Operand frontend::SymbolTable::get_operand(string id) const
{
    return get_ste(id).operand;
}

frontend::STE frontend::SymbolTable::get_ste(string id) const
{
    for (auto i = scope_stack.rbegin(); i != scope_stack.rend(); i++)
    {
        if (i->table.find(id) != i->table.end())
        {
            return i->table.at(id);
        }
    }
}

frontend::Analyzer::Analyzer() : tmp_cnt(0), symbol_table()
{
    symbol_table.add_scope();
}

ir::Program frontend::Analyzer::get_ir_program(CompUnit *root)
{
    ir::Program program;
    analysisCompUnit(root);
    for (auto p : symbol_table.scope_stack.back().table)
    {
        if (p.second.dimension.size())
        {
            int size = std::accumulate(p.second.dimension.begin(), p.second.dimension.end(), 1, std::multiplies<int>());
            program.globalVal.push_back({p.second.operand, size});
        }
        program.globalVal.push_back({p.second.operand});
    }
    ir::Function g = Function("_global", ir::Type::null);
    for (auto i : g_init_inst)
    {
        g.addInst(i);
    }

    program.addFunction(g);
    for (auto f : symbol_table.functions)
    {
        if (f.second->name == "main")
        {
            auto global_func_operand = Operand("_global", ir::Type::null);
            auto temp = Operand();
            f.second->InstVec.insert(f.second->InstVec.begin(), new ir::CallInst(global_func_operand, temp));
        }
        program.addFunction(*f.second);
    }
    return program;
}

// CompUnit -> (Decl | FuncDef) [CompUnit]
void frontend::Analyzer::analysisCompUnit(CompUnit *root)
{
    if (NODE_IS(DECL, 0))
    {
        // ANALYSIS(decl, Decl, 0);
        TODO;
    }
    else
    {
        auto buffer = Function();
        ANALYSIS(funcdef, FuncDef, 0);
        symbol_table.functions[funcdef->n] = &buffer;
    }

    if (root->children.size() == 2)
    {
        auto node = dynamic_cast<CompUnit *>(root->children[1]); 
        analysisCompUnit(node);
    }
}

// FuncDef -> FuncType Ident '(' [FuncFParams] ')' Block
void frontend::Analyzer::analysisFuncDef(FuncDef *root, ir::Function &function)
{
    auto buffer = function;
    ANALYSIS(functype, FuncType, 0);

    GET_CHILD_PTR(ident, Term, 1);
    function.name = ident->token.value;

    if (NODE_IS(TERMINAL, 3))
    {
        ANALYSIS(block, Block, 4);
    }
    else
    {
        TODO;
    }
    // else
    // {
    //     ANALYSIS(funcparams, FuncFParams, 3);
    // }
}

// FuncType -> 'void' | 'int' | 'float'
void frontend::Analyzer::analysisFuncType(FuncType *root, ir::Function &function)
{
    GET_CHILD_PTR(term, Term, 0);
    switch (term->token.type)
    {
    case TokenType::VOIDTK:
        function.returnType = ir::Type::null;
        break;
    case TokenType::INTTK:
        function.returnType = ir::Type::Int;
        break;
    case TokenType::FLOATTK:
        function.returnType = ir::Type::Float;
        break;
    default:
        break;
    }
}

// Block -> '{' { BlockItem } '}'
void frontend::Analyzer::analysisBlock(Block *root, ir::Function &function)
{
    auto buffer = std::vector<Instruction *>();
    symbol_table.add_scope();
    for (auto i = 1; i < root->children.size() - 1; i++)
    {
        ANALYSIS(blockitem, BlockItem, i);
        function.InstVec.insert(function.InstVec.end(), buffer.begin(), buffer.end());
    }
    symbol_table.exit_scope();
}

// BlockItem -> Decl | Stmt
void frontend::Analyzer::analysisBlockItem(BlockItem *root, std::vector<Instruction *> &instructions)
{
    auto buffer = instructions;
    if (NODE_IS(DECL, 0))
    {
        // ANALYSIS(decl, Decl, 0);
        TODO;
    }
    else
    {
        ANALYSIS(stmt, Stmt, 0);
    }
}

// Stmt -> LVal '=' Exp ';'                         first = { Ident }
//       | Block                                    first = { '{' }
//       | 'if' '(' Cond ')' Stmt [ 'else' Stmt ]   first = { 'if' }
//       | 'while' '(' Cond ')' Stmt                first = { 'while' }
//       | 'break' ';'                              first = { 'break' }
//       | 'continue' ';'                           first = { 'continue' }
//       | 'return' [Exp] ';'                       first = { 'return' }
//       | [Exp] ';'                                first = { Exp / ';' }
void frontend::Analyzer::analysisStmt(Stmt *root, std::vector<Instruction *> &instructions)
{
    auto buffer = instructions;
    auto i = Instruction();
    if (NODE_IS(TERMINAL, 0))
    {
        GET_CHILD_PTR(term, Term, 0);
        switch (term->token.type)
        {
        // case TokenType::IFTK:
        //     ANALYSIS(cond, Cond, 2);
        //     ANALYSIS(stmt1, Stmt, 4);
        //     if(NODE_IS(TERMINAL, 6)){
        //         ANALYSIS(stmt2, Stmt, 6);
        //     }
        //     break;
        // case TokenType::WHILETK:
        //     ANALYSIS(cond, Cond, 2);
        //     ANALYSIS(stmt, Stmt, 4);
        //     break;
        // case TokenType::BREAKTK:
        //     break;
        // case TokenType::CONTINUETK:
        //     break;
        case TokenType::RETURNTK:
            i.op = ir::Operator::_return;
            if (!(NODE_IS(TERMINAL, 1)))
            {
                ANALYSIS(exp, Exp, 1);
                // Exp.is_computable
                // Exp.v
                // Exp.t
                // TODO
                i.op1 = exp->v;
            }
            break;
        default:
            break;
        }
    }
    buffer.push_back(&i);
}

// Exp -> AddExp
// Exp.is_computable
// Exp.v
// Exp.t
void frontend::Analyzer::analysisExp(Exp *root, std::vector<Instruction *> &instructions)
{
    auto buffer = instructions;
    ANALYSIS(addexp, AddExp, 0);
    COPY_EXP_NODE(addexp, root);
}

// AddExp -> MulExp { ('+' | '-') MulExp }
// AddExp.is_computable
// AddExp.v
// AddExp.t
void frontend::Analyzer::analysisAddExp(AddExp *root, std::vector<Instruction *> &instructions)
{
    auto buffer = instructions;
    ANALYSIS(mulexp, MulExp, 0);
    COPY_EXP_NODE(mulexp, root); // TODO

    //{ ('+' | '-') MulExp } TODO
}

// MulExp -> UnaryExp { ('*' | '/' | '%') UnaryExp }
// MulExp.is_computable
// MulExp.v
// MulExp.t
void frontend::Analyzer::analysisMulExp(MulExp *root, std::vector<Instruction *> &instructions)
{
    auto buffer = instructions;
    ANALYSIS(unaryexp, UnaryExp, 0);
    COPY_EXP_NODE(unaryexp, root); // TODO

    // { ('*' | '/' | '%') UnaryExp } TODO
}

// UnaryExp -> PrimaryExp
//           | Ident '(' [FuncRParams] ')'
//           | UnaryOp UnaryExp
// UnaryExp.is_computable
// UnaryExp.v
// UnaryExp.t
void frontend::Analyzer::analysisUnaryExp(UnaryExp *root, std::vector<Instruction *> &instructions)
{
    auto buffer = instructions;
    if (NODE_IS(PRIMARYEXP, 0))
    {
        ANALYSIS(primaryexp, PrimaryExp, 0);
        COPY_EXP_NODE(primaryexp, root);
    }
    // else if (NODE_IS(TERMINAL, 0))
    // {
    //     switch (root->children[0]->type)
    //     {
    //     case TokenType::PLUS:
    //     case TokenType::MINU:
    //     case TokenType::NOT:
    //         ANALYSIS(unaryexp, UnaryExp, 1);
    //         COPY_EXP_NODE(unaryexp, root);
    //         break;
    //     default:
    //         break;
    //     }
    // }
    // else
    // {
    //     GET_CHILD_PTR(ident, Ident, 0);
    //     GET_CHILD_PTR(funcrparams, FuncRParams, 2);
    //     // TODO
    // }
}

// PrimaryExp -> '(' Exp ')' | LVal | Number
// PrimaryExp.is_computable
// PrimaryExp.v
// PrimaryExp.t
void frontend::Analyzer::analysisPrimaryExp(PrimaryExp *root,  vector<ir::Instruction *> &instruction)
{
    auto buffer = instruction;
    if (NODE_IS(NUMBER, 0))
    {
        ANALYSIS(number, Number, 0);
        root->is_computable = true;
        root->v = number->v;
        root->t = number->t;
    }
    // else if(NODE_IS(LVal,0)){
    //     ANALYSIS(lval, LVal, 0);
    //     root->is_computable = true;
    //     root->v = lval->v;
    //     root->t = lval->t;
    // }
    // else{
    //     ANALYSIS(exp, Exp, 1);
    //     root->is_computable = exp->is_computable;
    //     root->v = exp->v;
    //     root->t = exp->t;
    // }
}

// Number -> IntConst | floatConst
// Number.is_computable = true;
// Number.v
// Number.t
void frontend::Analyzer::analysisNumber(Number *root,  vector<ir::Instruction *> &instruction)
{
    GET_CHILD_PTR(term, Term, 0);
    switch (term->token.type)
    {
    case TokenType::INTLTR:
        root->v = term->token.value;
        root->t = Type::IntLiteral;
        break;
    case TokenType::FLOATLTR:
        root->v = term->token.value;
        root->t = Type::FloatLiteral;
        break;
    default:
        break;
    }
}