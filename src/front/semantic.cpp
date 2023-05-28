#include "front/semantic.h"

#include <cassert>
#include <numeric>
#include <iostream>

using ir::Function;
using ir::Instruction;
using ir::Operand;
using ir::Operator;

#define TODO assert(0 && "TODO");

// 获取一个树节点的指定类型子节点的指针，并且在获取失败的情况下，使用 assert 断言来停止程序的执行
// node->type == NodeType::TERMINAL
#define GET_CHILD_PTR(node, node_type, index)                     \
    auto node = dynamic_cast<node_type *>(root->children[index]); \
    assert(node);

// 获取一个树节点的指定类型子节点的指针，并调用一个名为 analysis<type> 的函数来对这个子节点进行分析
#define ANALYSIS(node, type, index)                          \
    auto node = dynamic_cast<type *>(root->children[index]); \
    assert(node);                                            \
    analysis##type(node, buffer);

#define ANALYSIS_NO_BUFFER(node, type, index)                \
    auto node = dynamic_cast<type *>(root->children[index]); \
    assert(node);                                            \
    analysis##type(node);

// 将一个表达式节点的信息复制到另一个表达式节点中
#define COPY_EXP_NODE(from, to)              \
    to->is_computable = from->is_computable; \
    to->v = from->v;                         \
    to->t = from->t;

#define NODE_IS(node_type, index) root->children[index]->type == NodeType::node_type

void log(frontend::Term *node)
{
    std::cout << "TERM: " << toString(node->token.type) << "\t" << node->token.value << '\n';
}

ir::Type to_const(ir::Type type)
{
    switch (type)
    {
    case ir::Type::Int:
    case ir::Type::IntPtr:
        return ir::Type::IntLiteral;
    case ir::Type::Float:
    case ir::Type::FloatPtr:
        return ir::Type::FloatLiteral;
    default:
        return type;
    }
}

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

void frontend::SymbolTable::add_operand(std::string name, STE ste)
{
    scope_stack.back().table[name] = ste;
}

ir::Operand frontend::Analyzer::get_temp(ir::Type type = ir::Type::null)
{
    return ir::Operand("_t" + std::to_string(tmp_cnt++), type);
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

    // 添加全局变量
    for (auto &p : symbol_table.scope_stack.back().table) // 遍历最外层作用域的符号表
    {
        if (p.second.dimension.size()) // 如果是数组
        {
            // 计算数组大小
            int size = std::accumulate(p.second.dimension.begin(), p.second.dimension.end(), 1, std::multiplies<int>());
            // 添加数组
            program.globalVal.push_back({p.second.operand, size});
            break;
        }
        // 添加变量
        program.globalVal.push_back({p.second.operand});
    }

    // 把全局变量的初始化指令添加到 _global 函数中
    ir::Function g = Function("_global", ir::Type::null);
    for (auto &i : g_init_inst) // 遍历全局变量的初始化指令
    {
        g.addInst(i);
    }
    g.addInst(new ir::Instruction({}, {}, {}, ir::Operator::_return));
    program.addFunction(g);

    auto call_global = new ir::CallInst(Operand("_global", ir::Type::null), Operand());

    // 把函数添加到 ir::Program 中
    for (auto &f : symbol_table.functions)
    {
        if (f.second->name == "main") // 如果是main函数
        {
            // 在main函数前面添加一个调用 _global 函数的指令
            f.second->InstVec.insert(f.second->InstVec.begin(), call_global);
        }
        program.functions.push_back(*f.second);
    }
    return program;
}

// CompUnit -> (Decl | FuncDef) [CompUnit]
void frontend::Analyzer::analysisCompUnit(CompUnit *root)
{
    if (NODE_IS(DECL, 0)) // 如果是声明
    {
        // 生成全局变量的初始化指令
        GET_CHILD_PTR(decl, Decl, 0);
        analysisDecl(decl, g_init_inst);
    }
    else // 如果是函数定义
    {
        // 生成一个新的函数
        auto a_new_func = Function();
        GET_CHILD_PTR(funcdef, FuncDef, 0);
        analysisFuncDef(funcdef, a_new_func);
        // 把函数添加到符号表中
        symbol_table.functions[a_new_func.name] = new Function(a_new_func);
    }

    if (root->children.size() == 2)
    {
        GET_CHILD_PTR(compunit, CompUnit, 1);
        analysisCompUnit(compunit);
    }
}

// Decl -> ConstDecl | VarDecl
void frontend::Analyzer::analysisDecl(Decl *root, vector<ir::Instruction *> &instructions)
{
    // std::cout << "analysisDecl" << std::endl;
    if (NODE_IS(VARDECL, 0)) // 如果是变量声明
    {
        // 向instructions中添加变量声明的指令
        GET_CHILD_PTR(vardecl, VarDecl, 0);
        analysisVarDecl(vardecl, instructions);
    }
    else // 如果是常量声明
    {
        // 向instructions中添加常量声明的指令
        GET_CHILD_PTR(constdecl, ConstDecl, 0);
        analysisConstDecl(constdecl, instructions);
    }
}

// ConstDecl -> 'const' BType ConstDef { ',' ConstDef } ';'
// ConstDecl.t
void frontend::Analyzer::analysisConstDecl(ConstDecl *root, vector<ir::Instruction *> &instructions)
{
    // std::cout << "analysisConstDecl" << std::endl;
    GET_CHILD_PTR(btype, BType, 1);
    analysisBType(btype);
    // root->t = to_const(btype->t);

    GET_CHILD_PTR(constdef, ConstDef, 2);
    analysisConstDef(constdef, instructions, root->t);

    for (size_t index = 4; index < root->children.size(); index += 2)
    {
        GET_CHILD_PTR(constdef, ConstDef, index);
        analysisConstDef(constdef, instructions, root->t);
    }
}

// ConstDef -> Ident { '[' ConstExp ']' } '=' ConstInitVal
// ConstDef.arr_name
void frontend::Analyzer::analysisConstDef(ConstDef *root, vector<ir::Instruction *> &instructions, ir::Type type)
{
    GET_CHILD_PTR(ident, Term, 0);
    root->arr_name = symbol_table.get_scoped_name(ident->token.value);

    std::vector<int> dimension;

    for (size_t index = 2; index < root->children.size(); index += 3)
    {
        if (!(NODE_IS(CONSTEXP, index)))
        {
            break;
        }
        GET_CHILD_PTR(constexp, ConstExp, index);
        analysisConstExp(constexp, instructions);
        dimension.push_back(std::stoi(constexp->v));
    }

    int size = std::accumulate(dimension.begin(), dimension.end(), 1, std::multiplies<int>());

    if (dimension.empty())
    {
        switch (type)
        {
        case ir::Type::Int:
            instructions.push_back(new ir::Instruction({"0", to_const(type)},
                                                       {},
                                                       {root->arr_name, type},
                                                       ir::Operator::def));
            break;
        case ir::Type::Float:
            instructions.push_back(new ir::Instruction({"0.0", to_const(type)},
                                                       {},
                                                       {root->arr_name, type},
                                                       ir::Operator::fdef));
            break;
        default:
            break;
        }
        type = to_const(type);
    }
    else
    {
        switch (type)
        {
        case ir::Type::Int:
            type = ir::Type::IntPtr;
            break;
        case ir::Type::Float:
            type = ir::Type::FloatPtr;
            break;
        default:
            break;
        }

        instructions.push_back(new ir::Instruction({std::to_string(size), ir::Type::IntLiteral},
                                                   {},
                                                   {root->arr_name, type},
                                                   ir::Operator::alloc));
    }

    GET_CHILD_PTR(constinitval, ConstInitVal, root->children.size() - 1);
    constinitval->t = type;
    constinitval->v = root->arr_name;
    analysisConstInitVal(constinitval, instructions);

    // !2!
    std::cout << "ConstDef: " << ident->token.value + " " + constinitval->v + " " + toString(to_const(type)) << std::endl;
    // Operand(symbol_table.get_scoped_name(ident->token.value), type)
    symbol_table.add_operand(ident->token.value,
                             {Operand(constinitval->v, type),
                              dimension});
}

// ConstInitVal -> ConstExp | '{' [ ConstInitVal { ',' ConstInitVal } ] '}'
// ConstInitVal.v
// ConstInitVal.t
void frontend::Analyzer::analysisConstInitVal(ConstInitVal *root, vector<ir::Instruction *> &instructions)
{
    if (NODE_IS(CONSTEXP, 0))
    {
        GET_CHILD_PTR(constexp, ConstExp, 0);
        constexp->v = get_temp().name;
        analysisConstExp(constexp, instructions);
        instructions.push_back(new ir::Instruction({constexp->v, constexp->t},
                                                   {},
                                                   {root->v, root->t},
                                                   ir::Operator::mov));
        root->v = constexp->v;

        root->t = constexp->t;
        // std::cout << "analysisConstInitVal: " + constexp->v + " to " + root->v << std::endl;
    }
    else
    {
        int insert_index = 0;
        for (size_t index = 1; index < root->children.size() - 1; index += 2)
        {
            if (NODE_IS(CONSTINITVAL, index))
            {
                GET_CHILD_PTR(constinitval, ConstInitVal, index);
                constinitval->v = get_temp().name;
                analysisConstInitVal(constinitval, instructions);
                // !3
                // std::cout << toString(constinitval->t) << std::endl;
                instructions.push_back(new ir::Instruction({root->v, root->t},
                                                           {std::to_string(insert_index), ir::Type::IntLiteral},
                                                           {constinitval->v, constinitval->t},
                                                           ir::Operator::store));
                insert_index++;
            }
            else
            {
                break;
            }
        }
    }
}

// VarDecl -> BType VarDef { ',' VarDef } ';'
// VarDecl.t
void frontend::Analyzer::analysisVarDecl(VarDecl *root, vector<ir::Instruction *> &instructions)
{
    // std::cout << "analysisVarDecl" << std::endl;
    GET_CHILD_PTR(btype, BType, 0);
    analysisBType(btype);
    root->t = btype->t;

    GET_CHILD_PTR(vardef, VarDef, 1);
    analysisVarDef(vardef, instructions, btype->t);

    for (size_t index = 3; index < root->children.size(); index += 2)
    {
        GET_CHILD_PTR(vardef, VarDef, index);
        analysisVarDef(vardef, instructions, btype->t);
    }
}

// BType -> 'int' | 'float'
// BType.t
void frontend::Analyzer::analysisBType(BType *root)
{
    GET_CHILD_PTR(term, Term, 0);
    switch (term->token.type)
    {
    case TokenType::INTTK:
        root->t = ir::Type::Int;
        break;
    case TokenType::FLOATTK:
        root->t = ir::Type::Float;
        break;
    default:
        break;
    }
}

// VarDef -> Ident { '[' ConstExp ']' } [ '=' InitVal ]
// VarDef.arr_name
// Instruction(op1, op2, des, op);
// int a = 8; => def a, 8 第一个操作数为立即数或变量，第二个操作数不使用，结果为被赋值变量。
// float a = 8.0; => fdef a, 8.0
void frontend::Analyzer::analysisVarDef(VarDef *root, vector<ir::Instruction *> &instructions, ir::Type type)
{
    // 获取被赋值变量名, for des
    GET_CHILD_PTR(ident, Term, 0);

    root->arr_name = symbol_table.get_scoped_name(ident->token.value);

    std::vector<int> dimension;

    for (size_t index = 2; index < root->children.size(); index += 3)
    {
        if (!(NODE_IS(CONSTEXP, index)))
            break;
        GET_CHILD_PTR(constexp, ConstExp, index);
        analysisConstExp(constexp, instructions);
        std::cout << toString(constexp->t) + " " + constexp->v << std::endl;
        dimension.push_back(std::stoi(constexp->v));
    }

    // 计算数组大小
    int size = std::accumulate(dimension.begin(), dimension.end(), 1, std::multiplies<int>());

    if (dimension.empty()) // 如果非数组
    {
        // 向instructions中添加变量定义的指令
        switch (type)
        {
        case ir::Type::Int:
            instructions.push_back(new ir::Instruction({"0", to_const(type)},
                                                       {},
                                                       {root->arr_name, type},
                                                       ir::Operator::def));
            break;
        case ir::Type::Float:
            instructions.push_back(new ir::Instruction({"0.0", to_const(type)},
                                                       {},
                                                       {root->arr_name, type},
                                                       ir::Operator::fdef));
            break;
        default:
            break;
        }
    }
    else // 如果是数组
    {
        switch (type)
        {
        case ir::Type::Int:
            type = ir::Type::IntPtr;
            break;
        case ir::Type::Float:
            type = ir::Type::FloatPtr;
            break;
        default:
            assert(0 && "analysisVarDef");
            break;
        }
        // 向instructions中添加数组定义的指令
        // 内存分配指令，用于局部数组变量声明。第一个操作数为数组长度（非栈帧移动长度），第二个操作数不使用，结果为数组名，数组名被视为一个指针。
        instructions.push_back(new ir::Instruction({std::to_string(size), to_const(ir::Type::IntLiteral)},
                                                   {},
                                                   {root->arr_name, type},
                                                   ir::Operator::alloc));
    }

    if (NODE_IS(INITVAL, root->children.size() - 1)) // 如果有初始化值
    {
        GET_CHILD_PTR(initval, InitVal, root->children.size() - 1);
        initval->t = type;
        initval->v = root->arr_name;
        // 向instructions中添加初始化值的指令
        analysisInitVal(initval, instructions);
    }

    symbol_table.add_operand(ident->token.value,
                             {Operand(symbol_table.get_scoped_name(ident->token.value), type),
                              dimension});
}

// ConstExp -> AddExp
// ConstExp.is_computable: true
// ConstExp.v
// ConstExp.t
void frontend::Analyzer::analysisConstExp(ConstExp *root, vector<ir::Instruction *> &instructions)
{
    GET_CHILD_PTR(addexp, AddExp, 0);
    COPY_EXP_NODE(root, addexp);
    analysisAddExp(addexp, instructions);
    COPY_EXP_NODE(addexp, root);
}

// InitVal -> Exp | '{' [ InitVal { ',' InitVal } ] '}'
// InitVal.is_computable
// InitVal.v
// InitVal.t
void frontend::Analyzer::analysisInitVal(InitVal *root, vector<ir::Instruction *> &instructions)
{
    if (NODE_IS(EXP, 0))
    {
        GET_CHILD_PTR(exp, Exp, 0);
        exp->v = get_temp().name;
        analysisExp(exp, instructions);

        // mov:第一个操作数为赋值变量，第二个操作数不使用，结果为被赋值变量

        instructions.push_back(new ir::Instruction({exp->v, exp->t},
                                                   {},
                                                   {root->v, root->t},
                                                   ir::Operator::mov));

        // std::cout << "mov: " + toString(exp->t) + " " + exp->v + " " + toString(root->t) + " " + root->v << std::endl;
        if (exp->t == ir::Type::IntLiteral)
        {
            root->v = exp->v;
            root->t = exp->t;
            // return;
        }
    }
    else
    {
        // std::cout << root->children.size() << std::endl;
        int insert_index = 0;
        for (size_t index = 1; index < root->children.size(); index += 2)
        {
            if (NODE_IS(INITVAL, index))
            {
                GET_CHILD_PTR(initval, InitVal, index);
                initval->v = get_temp().name;
                analysisInitVal(initval, instructions);
                // arr[2] = 3; => store 3, arr, 2
                instructions.push_back(new ir::Instruction({root->v, root->t},
                                                           {std::to_string(insert_index), ir::Type::IntLiteral},
                                                           {initval->v, initval->t},
                                                           ir::Operator::store));
                insert_index += 1;
            }
            else
            {
                break;
            }
        }
    }
}

// FuncDef -> FuncType Ident '(' [FuncFParams] ')' Block
// 生成函数{name, returnType, ParameterList, InstVec}
// Function(name, returnType);
// Function(name, ParameterList(vector<Operand>), returnType);
// Function.addInst(Instruction* inst);
void frontend::Analyzer::analysisFuncDef(FuncDef *root, ir::Function &function)
{
    // 生成函数返回值类型
    GET_CHILD_PTR(functype, FuncType, 0);
    analysisFuncType(functype, function.returnType);

    // 生成函数名
    GET_CHILD_PTR(ident, Term, 1);
    function.name = ident->token.value;

    if (NODE_IS(FUNCFPARAMS, 3)) // 如果有参数
    {
        // 生成函数参数列表
        GET_CHILD_PTR(funcfparams, FuncFParams, 3);
        analysisFuncFParams(funcfparams, function.ParameterList);
    }

    // 生成函数体指令到InstVec
    GET_CHILD_PTR(block, Block, root->children.size() - 1);
    analysisBlock(block, function.InstVec);
}

// FuncType -> 'void' | 'int' | 'float'
void frontend::Analyzer::analysisFuncType(FuncType *root, ir::Type &returnType)
{
    GET_CHILD_PTR(term, Term, 0);

    switch (term->token.type)
    {
    case TokenType::VOIDTK:
        returnType = ir::Type::null;
        break;
    case TokenType::INTTK:
        returnType = ir::Type::Int;
        break;
    case TokenType::FLOATTK:
        returnType = ir::Type::Float;
        break;
    default:
        break;
    }
}

// FuncFParams -> FuncFParam { ',' FuncFParam }
void frontend::Analyzer::analysisFuncFParams(FuncFParams *root, vector<ir::Operand> &params)
{
    for (size_t i = 0; i < root->children.size(); i++)
    {
        if (NODE_IS(FUNCFPARAM, i))
        {
            GET_CHILD_PTR(funcfparam, FuncFParam, i);
            analysisFuncFParam(funcfparam, params);
        }
    }
}

// FuncFParam -> BType Ident ['[' ']' { '[' Exp ']' }]
void frontend::Analyzer::analysisFuncFParam(FuncFParam *root, vector<ir::Operand> &params)
{
    GET_CHILD_PTR(btype, BType, 0);
    analysisBType(btype);
    auto type = btype->t;

    GET_CHILD_PTR(ident, Term, 1);

    vector<int> dimension;
    if (root->children.size() > 2)
    {
        if (root->children.size() == 4) // 维度为1
        {
            switch (type)
            {
            case ir::Type::Int:
            case ir::Type::IntLiteral:
                type = ir::Type::IntPtr;
                break;
            case ir::Type::Float:
            case ir::Type::FloatLiteral:
                type = ir::Type::FloatPtr;
                break;
            default:
                break;
            }
            dimension.push_back(-1);
        }
        else
        {
            TODO;
        }
    }

    params.push_back(Operand(symbol_table.get_scoped_name(ident->token.value), type));
    symbol_table.add_operand(ident->token.value,
                             {Operand(symbol_table.get_scoped_name(ident->token.value), type), dimension});
}

// Block -> '{' { BlockItem } '}'
void frontend::Analyzer::analysisBlock(Block *root, vector<ir::Instruction *> &instructions)
{
    // 进入新的作用域（出现‘{’就进入新的作用域）
    symbol_table.add_scope();
    for (size_t i = 1; i < root->children.size() - 1; i++)
    {
        // 生成指令到instructions中
        GET_CHILD_PTR(blockitem, BlockItem, i);
        analysisBlockItem(blockitem, instructions);
    }
    // 退出作用域（出现‘}’就退出作用域）
    symbol_table.exit_scope();
}

// BlockItem -> Decl | Stmt
void frontend::Analyzer::analysisBlockItem(BlockItem *root, std::vector<Instruction *> &instructions)
{
    if (NODE_IS(DECL, 0)) // 如果是声明
    {
        GET_CHILD_PTR(decl, Decl, 0);
        analysisDecl(decl, instructions);
    }
    else // 如果是语句
    {
        GET_CHILD_PTR(stmt, Stmt, 0);
        analysisStmt(stmt, instructions);
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
    if (NODE_IS(LVAL, 0)) // 如果是赋值语句
    {
        GET_CHILD_PTR(lval, LVal, 0);
        // 获取左值
        analysisLVal(lval, instructions);

        GET_CHILD_PTR(exp, Exp, 2);
        // exp->v = lval->v;
        exp->v = get_temp().name;

        // 获取右值
        analysisExp(exp, instructions);

        // std::cout << "进行赋值:" << toString(lval->t) << " " << lval->v << " = " << toString(exp->t) << " " << exp->v
        //           << std::endl; // todelete
        switch (exp->t)
        {
        case ir::Type::Int:
        case ir::Type::IntLiteral:
            switch (lval->t)
            {
            case ir::Type::Float:
            case ir::Type::FloatPtr: // for 数组
                instructions.push_back(new ir::Instruction({exp->v, ir::Type::Float}, {}, {exp->v, exp->t}, ir::Operator::cvt_i2f));
                break;
            default:
                break;
            }
            break;
        case ir::Type::Float:
        case ir::Type::FloatLiteral:
            switch (lval->t)
            {
            case ir::Type::Int:
            case ir::Type::IntPtr: // for 数组
                instructions.push_back(new ir::Instruction({exp->v, ir::Type::Int}, {}, {exp->v, exp->t}, ir::Operator::cvt_f2i));
                break;
            default:
                break;
            }
            break;
        default:
            assert(0);
            break;
        }

        if (lval->i) // 如果是数组
        {
            // 第一个操作数为数组名，第二个操作数为要存数所在数组下标，目的操作数为存入的数。示例如下：
            //      arr[2] = 3; => store 3, arr, 2
            instructions.push_back(new ir::Instruction({exp->v, exp->t},
                                                       {std::to_string(lval->i), ir::Type::IntLiteral},
                                                       {lval->v, lval->t},
                                                       ir::Operator::store));
        }
        else // 如果是非数组 lval->i == 0
        {
            instructions.push_back(new ir::Instruction({exp->v, exp->t},
                                                       {},
                                                       {lval->v, lval->t},
                                                       ir::Operator::mov));
        }
    }
    else if (NODE_IS(BLOCK, 0)) // 如果是复合语句
    {
        GET_CHILD_PTR(block, Block, 0);
        analysisBlock(block, instructions);
    }
    else if (NODE_IS(EXP, 0)) // 如果是表达式语句
    {
        GET_CHILD_PTR(exp, Exp, 0);
        analysisExp(exp, instructions);
    }
    else if (NODE_IS(TERMINAL, 0)) // 如果是标识符
    {
        GET_CHILD_PTR(term, Term, 0);
        switch (term->token.type)
        {
        case TokenType::IFTK: // 如果是if语句
        {                     // 'if' '(' Cond ')' Stmt [ 'else' Stmt ]
            GET_CHILD_PTR(cond, Cond, 2);
            cond->v = get_temp().name;
            analysisCond(cond, instructions);
            // op1为跳转条件，其为整形变量或type = Type::null的变量，当为整形变量时表示条件跳转（值不等于0发生跳转），否则为无条件跳转。
            // op2不使用，des应为整形，其值为跳转相对目前pc的偏移量

            // 当条件为真时跳转到if_true_stmt（下下条指令）
            // 否则继续执行下一条指令，下一条指令也应为跳转指令，指向if_true_stmt结束之后的指令
            instructions.push_back(new ir::Instruction({cond->v, cond->t},
                                                       {},
                                                       {"2", ir::Type::IntLiteral},
                                                       ir::Operator::_goto));

            vector<ir::Instruction *> if_true_instructions;
            GET_CHILD_PTR(if_true_stmt, Stmt, 4);
            analysisStmt(if_true_stmt, if_true_instructions);

            vector<ir::Instruction *> if_false_instructions;
            if (root->children.size() == 7)
            {
                GET_CHILD_PTR(if_false_stmt, Stmt, 6);
                analysisStmt(if_false_stmt, if_false_instructions);
                // 执行完if_true_stmt后跳转到if_false_stmt后面
                if_true_instructions.push_back(new ir::Instruction({},
                                                                   {},
                                                                   {std::to_string(if_false_instructions.size() + 1), ir::Type::IntLiteral},
                                                                   ir::Operator::_goto));
            }

            // 执行完if_true_stmt后跳转到if_false_stmt/if-else代码段（此时无if_false_stmt）后面
            instructions.push_back(new ir::Instruction({},
                                                       {},
                                                       {std::to_string(if_true_instructions.size() + 1), ir::Type::IntLiteral},
                                                       ir::Operator::_goto));
            instructions.insert(instructions.end(), if_true_instructions.begin(), if_true_instructions.end());
            instructions.insert(instructions.end(), if_false_instructions.begin(), if_false_instructions.end());
            break;
        }
        case TokenType::WHILETK: // 如果是while语句
        {                        // 'while' '(' Cond ')' Stmt
            GET_CHILD_PTR(cond, Cond, 2);
            cond->v = get_temp().name;

            vector<ir::Instruction *> cond_instructions;
            analysisCond(cond, cond_instructions);
            // 先执行cond，如果为真则执行while_stmt，跳转到下下条（跳过下一条指令）
            // 否则执行下一条，跳转到while_stmt后面
            instructions.insert(instructions.end(), cond_instructions.begin(), cond_instructions.end());
            instructions.push_back(new ir::Instruction({cond->v, cond->t},
                                                       {},
                                                       {"2", ir::Type::IntLiteral},
                                                       ir::Operator::_goto));

            GET_CHILD_PTR(stmt, Stmt, 4);
            vector<ir::Instruction *> while_instructions;
            analysisStmt(stmt, while_instructions);

            // 执行完while_stmt后跳回到cond的第一条
            while_instructions.push_back(new ir::Instruction({},
                                                             {},
                                                             {std::to_string(-int(while_instructions.size() + 2 + cond_instructions.size())), ir::Type::IntLiteral},
                                                             // +2是因为while_stmt前面分别还有2条跳转指令
                                                             ir::Operator::_goto));

            // 跳转到while_stmt后面
            instructions.push_back(new ir::Instruction({},
                                                       {},
                                                       {std::to_string(while_instructions.size() + 1), ir::Type::IntLiteral},
                                                       ir::Operator::_goto));
            for (size_t i = 0; i < while_instructions.size(); i++)
            {
                if (while_instructions[i]->op == ir::Operator::__unuse__ && while_instructions[i]->op1.name == "1")
                {
                    while_instructions[i] = new ir::Instruction({},
                                                                {},
                                                                {std::to_string(int(while_instructions.size()) - i), ir::Type::IntLiteral},
                                                                ir::Operator::_goto);
                }
                else if (while_instructions[i]->op == ir::Operator::__unuse__ && while_instructions[i]->op1.name == "2")
                {
                    while_instructions[i] = new ir::Instruction({},
                                                                {},
                                                                {std::to_string(-int(i + 2 + cond_instructions.size())), ir::Type::IntLiteral},
                                                                ir::Operator::_goto);
                }
            }

            instructions.insert(instructions.end(), while_instructions.begin(), while_instructions.end());
            break;
        }
        case TokenType::BREAKTK: // 如果是break语句
        {                        // 'break' ';'
            instructions.push_back(new ir::Instruction({"1", ir::Type::IntLiteral},
                                                       {},
                                                       {},
                                                       ir::Operator::__unuse__));
            break;
        }
        case TokenType::CONTINUETK: // 如果是continue语句
        {                           // 'continue' ';'
            instructions.push_back(new ir::Instruction({"2", ir::Type::IntLiteral},
                                                       {},
                                                       {},
                                                       ir::Operator::__unuse__));
            break;
        }
        case TokenType::RETURNTK: // 如果是return语句
        {                         // 第一个操作数为返回值，第二个操作数与结果不使用。示例如下：
            // return a; => return a
            if (NODE_IS(EXP, 1)) // 如果有返回值
            {
                GET_CHILD_PTR(exp, Exp, 1);
                exp->v = get_temp().name;
                analysisExp(exp, instructions);
                instructions.push_back(new ir::Instruction({exp->v, exp->t},
                                                           {},
                                                           {},
                                                           ir::Operator::_return));
            }
            else
            {
                instructions.push_back(new ir::Instruction({"0", ir::Type::null},
                                                           {},
                                                           {},
                                                           ir::Operator::_return));
            }
            break;
        }
        default:
            break;
        }
    }
}

// Cond -> LOrExp
// Cond.is_computable
// Cond.v
// Cond.t
void frontend::Analyzer::analysisCond(Cond *root, std::vector<Instruction *> &instructions)
{
    GET_CHILD_PTR(lorexp, LOrExp, 0);
    COPY_EXP_NODE(root, lorexp);
    analysisLOrExp(lorexp, instructions);
    COPY_EXP_NODE(lorexp, root);

    // switch (root->t)
    // {
    // case ir::Type::IntLiteral:
    //     root->v = (root->v == "0" ? "0" : "1");
    //     break;
    // case ir::Type::Int:
    //     root->v = (std::stoi(root->v) == 0 ? "0" : "1");
    //     break;
    // case ir::Type::FloatLiteral:
    //     root->v = (root->v == "0.0" ? "0" : "1");
    //     break;
    // case ir::Type::Float:
    //     root->v = (std::stof(root->v) == 0.0 ? "0" : "1");
    //     break;
    // default:
    //     assert(false);
    //     break;
    // }
}

// LOrExp -> LAndExp [ '||' LOrExp ]
// LOrExp.is_computable
// LOrExp.v
// LOrExp.t
void frontend::Analyzer::analysisLOrExp(LOrExp *root, std::vector<Instruction *> &instructions)
{
    // // 初始化为1，表示true
    // instructions.push_back(new ir::Instruction({"1", ir::Type::IntLiteral},
    //                                            {},
    //                                            {root->v, ir::Type::Int},
    //                                            ir::Operator::def));
    GET_CHILD_PTR(landexp, LAndExp, 0);
    COPY_EXP_NODE(root, landexp);
    analysisLAndExp(landexp, instructions);
    COPY_EXP_NODE(landexp, root);

    for (size_t i = 2; i < root->children.size(); i += 2) // 如果有多个LOrExp
    {
        GET_CHILD_PTR(lorexp, LOrExp, i);
        analysisLOrExp(lorexp, instructions);

        GET_CHILD_PTR(term, Term, i - 1);
        auto temp_name = get_temp().name;

        instructions.push_back(new ir::Instruction({root->v, root->t},
                                                   {lorexp->v, lorexp->t},
                                                   {temp_name, ir::Type::Int},
                                                   ir::Operator::_or));

        root->v = temp_name;
        root->t = ir::Type::Int;
    }
}

// LAndExp -> EqExp [ '&&' LAndExp ]
// LAndExp.is_computable
// LAndExp.v
// LAndExp.t
void frontend::Analyzer::analysisLAndExp(LAndExp *root, vector<ir::Instruction *> &instructions)
{
    // std::cout << "LAndExp: " + toString(root->t) + " " + root->v << std::endl;
    GET_CHILD_PTR(eqexp, EqExp, 0);
    COPY_EXP_NODE(root, eqexp);
    analysisEqExp(eqexp, instructions);
    COPY_EXP_NODE(eqexp, root);

    // std::cout << "LAndExp: " + toString(root->t) + " " + root->v << std::endl;

    for (size_t i = 2; i < root->children.size(); i += 2) // 如果有多个LAndExp
    {
        GET_CHILD_PTR(landexp, LAndExp, i);
        analysisLAndExp(landexp, instructions);

        GET_CHILD_PTR(term, Term, i - 1);
        auto temp_name = get_temp().name;

        instructions.push_back(new ir::Instruction({root->v, root->t},
                                                   {landexp->v, landexp->t},
                                                   {temp_name, ir::Type::Int},
                                                   ir::Operator::_and));

        root->v = temp_name;
        root->t = ir::Type::Int;
    }
}

// EqExp -> RelExp { ('==' | '!=') RelExp }
// EqExp.is_computable
// EqExp.v
// EqExp.t
void frontend::Analyzer::analysisEqExp(EqExp *root, vector<ir::Instruction *> &instructions)
{
    GET_CHILD_PTR(relexp, RelExp, 0);
    COPY_EXP_NODE(root, relexp);
    analysisRelExp(relexp, instructions);
    COPY_EXP_NODE(relexp, root);
    // std::cout << toString(root->t)+ " " << root->v << std::endl;

    for (size_t i = 2; i < root->children.size(); i += 2) // 如果有多个RelExp
    {
        GET_CHILD_PTR(relexp, RelExp, i);
        analysisRelExp(relexp, instructions);
        // std::cout << toString(relexp->t)+ " " << relexp->v << std::endl;

        GET_CHILD_PTR(term, Term, i - 1);
        auto temp_name = get_temp().name;

        switch (term->token.type) // TODO: 只考虑整型
        {
        case TokenType::EQL: // 整型变量==运算，逻辑运算结果用变量表示。
        {
            instructions.push_back(new ir::Instruction({root->v, root->t},
                                                       {relexp->v, relexp->t},
                                                       {temp_name, ir::Type::Int},
                                                       ir::Operator::eq));
            break;
        }
        case TokenType::NEQ:
        {
            instructions.push_back(new ir::Instruction({root->v, root->t},
                                                       {relexp->v, relexp->t},
                                                       {temp_name, ir::Type::Int},
                                                       ir::Operator::neq));
            break;
        }
        default:
            assert(false);
            break;
        }
        root->v = temp_name;
        root->t = ir::Type::Int;
    }
}

// RelExp -> AddExp { ('<' | '>' | '<=' | '>=') AddExp }
// RelExp.is_computable
// RelExp.v
// RelExp.t
void frontend::Analyzer::analysisRelExp(RelExp *root, vector<ir::Instruction *> &instructions)
{
    // std::cout << "RelExp: " + toString(root->t) + " " + root->v << std::endl;
    GET_CHILD_PTR(addexp, AddExp, 0);
    COPY_EXP_NODE(root, addexp);
    analysisAddExp(addexp, instructions);
    COPY_EXP_NODE(addexp, root);

    // std::cout << "RelExp: " + toString(root->t) + " " + root->v << std::endl;

    for (size_t i = 2; i < root->children.size(); i += 2) // 如果有多个AddExp
    {
        GET_CHILD_PTR(addexp, AddExp, i);
        analysisAddExp(addexp, instructions);

        GET_CHILD_PTR(term, Term, i - 1);

        if (root->is_computable && addexp->is_computable)
        {
            root->t = ir::Type::IntLiteral;
            switch (term->token.type)
            {
            case TokenType::LSS:
                root->v = std::to_string(std::stoi(root->v) < std::stoi(addexp->v));
                break;
            case TokenType::LEQ:
                root->v = std::to_string(std::stoi(root->v) <= std::stoi(addexp->v));
                break;
            case TokenType::GTR:
                root->v = std::to_string(std::stoi(root->v) > std::stoi(addexp->v));
                break;
            case TokenType::GEQ:
                root->v = std::to_string(std::stoi(root->v) >= std::stoi(addexp->v));
                break;
            default:
                break;
            }
        }
        else
        {
            auto temp_name = get_temp().name;
            switch (term->token.type) // TODO: 暂时只考虑整型
            {
            case TokenType::LSS: // 整型变量<运算，逻辑运算结果用变量表示
            {
                instructions.push_back(new ir::Instruction({root->v, root->t},
                                                           {addexp->v, addexp->t},
                                                           {temp_name, ir::Type::Int},
                                                           ir::Operator::lss));
                break;
            }
            case TokenType::LEQ: // 整型变量<=运算，逻辑运算结果用变量表示
            {
                instructions.push_back(new ir::Instruction({root->v, root->t},
                                                           {addexp->v, addexp->t},
                                                           {temp_name, ir::Type::Int},
                                                           ir::Operator::leq));
                break;
            }
            case TokenType::GTR: // 整型变量>运算，逻辑运算结果用变量表示
            {
                instructions.push_back(new ir::Instruction({root->v, root->t},
                                                           {addexp->v, addexp->t},
                                                           {temp_name, ir::Type::Int},
                                                           ir::Operator::gtr));
                break;
            }
            case TokenType::GEQ: // 整型变量>=运算，逻辑运算结果用变量表示
            {
                instructions.push_back(new ir::Instruction({root->v, root->t},
                                                           {addexp->v, addexp->t},
                                                           {temp_name, ir::Type::Int},
                                                           ir::Operator::geq));
                break;
            }
            default:
                assert(false);
                break;
            }
            root->v = temp_name;
            root->t = ir::Type::Int;
        }
    }
}

// Exp -> AddExp
// Exp.is_computable
// Exp.v
// Exp.t
void frontend::Analyzer::analysisExp(Exp *root, vector<ir::Instruction *> &instructions)
{
    GET_CHILD_PTR(addexp, AddExp, 0);
    // addexp->v = root->v;
    COPY_EXP_NODE(root, addexp);
    // std::cout << "in Exp " << root->v << std::endl;
    analysisAddExp(addexp, instructions);

    COPY_EXP_NODE(addexp, root);
    // std::cout << "in Exp " << root->v << std::endl;
}

// AddExp -> MulExp { ('+' | '-') MulExp }
// AddExp.is_computable
// AddExp.v
// AddExp.t
void frontend::Analyzer::analysisAddExp(AddExp *root, std::vector<Instruction *> &instructions)
{
    GET_CHILD_PTR(mulexp, MulExp, 0);
    COPY_EXP_NODE(root, mulexp);
    // std::cout << "in AddExp" << root->v << std::endl;
    analysisMulExp(mulexp, instructions);
    // std::cout << "analysisConstInitVal: " + mulexp->v << std::endl;

    COPY_EXP_NODE(mulexp, root);
    // std::cout << "in AddExp " << root->v << std::endl;
    // 如果有多个MulExp
    for (size_t i = 2; i < root->children.size(); i += 2)
    {
        GET_CHILD_PTR(mulexp, MulExp, i);
        // mulexp->v = get_temp().name;
        analysisMulExp(mulexp, instructions);
        auto temp_name = get_temp().name;
        GET_CHILD_PTR(term, Term, i - 1);

        switch (root->t)
        {
        case ir::Type::Int:
            switch (mulexp->t)
            {
            case ir::Type::Int:
            case ir::Type::IntLiteral:
                // std::cout << toString(root->t) + " + " + toString(mulexp->t) << std::endl;
                if (term->token.type == TokenType::PLUS)
                {
                    instructions.push_back(new ir::Instruction({root->v, root->t},
                                                               {mulexp->v, mulexp->t},
                                                               {temp_name, ir::Type::Int},
                                                               ir::Operator::add));
                }
                else
                {
                    instructions.push_back(new ir::Instruction({root->v, root->t},
                                                               {mulexp->v, mulexp->t},
                                                               {temp_name, ir::Type::Int},
                                                               ir::Operator::sub));
                }
                root->v = temp_name;
                break;

            default:
                break;
            }
            break;

        case ir::Type::IntLiteral:
            switch (mulexp->t)
            {
            case ir::Type::Int:
                if (term->token.type == TokenType::PLUS)
                {
                    instructions.push_back(new ir::Instruction({root->v, root->t},
                                                               {mulexp->v, mulexp->t},
                                                               {temp_name, ir::Type::Int},
                                                               ir::Operator::add));
                }
                else
                {
                    instructions.push_back(new ir::Instruction({root->v, root->t},
                                                               {mulexp->v, mulexp->t},
                                                               {temp_name, ir::Type::Int},
                                                               ir::Operator::sub));
                }
                root->v = temp_name;
                root->t = ir::Type::Int;
                break;
            case ir::Type::IntLiteral:
                if (term->token.type == TokenType::PLUS)
                {
                    // instructions.push_back(new ir::Instruction({root->v, root->t},
                    //                                            {mulexp->v, mulexp->t},
                    //                                            {temp_name, ir::Type::IntLiteral},
                    //                                            ir::Operator::add));

                    root->v = std::to_string(std::stoi(root->v) + std::stoi(mulexp->v));
                }
                else
                {
                    // instructions.push_back(new ir::Instruction({root->v, root->t},
                    //                                            {mulexp->v, mulexp->t},
                    //                                            {temp_name, ir::Type::IntLiteral},
                    //                                            ir::Operator::sub));
                    root->v = std::to_string(std::stoi(root->v) - std::stoi(mulexp->v));
                }
                break;
            default:
                break;
            }
            break;

        default:
            break;
        }
        // auto temp_name = get_temp().name;
        // if (term->token.type == TokenType::PLUS)
        // {
        //     instructions.push_back(new ir::Instruction({root->v, root->t},
        //                                                {mulexp->v, mulexp->t},
        //                                                {temp_name, ir::Type::Int},
        //                                                ir::Operator::add));
        // }
        // else
        // {
        //     instructions.push_back(new ir::Instruction({root->v, root->t},
        //                                                {mulexp->v, mulexp->t},
        //                                                {temp_name, ir::Type::Int},
        //                                                ir::Operator::sub));
        // }
        // root->v = temp_name;
        // root->t = ir::Type::Int;
    }
}

// MulExp -> UnaryExp { ('*' | '/' | '%') UnaryExp }
// MulExp.is_computable
// MulExp.v
// MulExp.t
void frontend::Analyzer::analysisMulExp(MulExp *root, std::vector<Instruction *> &instructions)
{
    GET_CHILD_PTR(unaryexp, UnaryExp, 0);
    COPY_EXP_NODE(root, unaryexp);
    analysisUnaryExp(unaryexp, instructions);
    // std::cout << "analysisConstInitVal: " + unaryexp->v << std::endl;

    COPY_EXP_NODE(unaryexp, root);

    // 如果有多个UnaryExp
    for (size_t i = 2; i < root->children.size(); i += 2)
    {
        GET_CHILD_PTR(unaryexp, UnaryExp, i);
        // unaryexp->v = get_temp().name;
        auto temp_name = get_temp().name;
        analysisUnaryExp(unaryexp, instructions);

        GET_CHILD_PTR(term, Term, i - 1);

        switch (root->t)
        {
        case ir::Type::Int:
            switch (unaryexp->t)
            {
            case ir::Type::Int:
            case ir::Type::IntLiteral:
                switch (term->token.type)
                {
                case TokenType::MULT:
                    instructions.push_back(new ir::Instruction({root->v, root->t},
                                                               {unaryexp->v, unaryexp->t},
                                                               {temp_name, ir::Type::Int},
                                                               ir::Operator::mul));
                    break;
                case TokenType::DIV:
                    instructions.push_back(new ir::Instruction({root->v, root->t},
                                                               {unaryexp->v, unaryexp->t},
                                                               {temp_name, ir::Type::Int},
                                                               ir::Operator::div));
                    break;
                case TokenType::MOD:
                    instructions.push_back(new ir::Instruction({root->v, root->t},
                                                               {unaryexp->v, unaryexp->t},
                                                               {temp_name, ir::Type::Int},
                                                               ir::Operator::mod));
                    break;
                default:
                    break;
                }
                root->v = temp_name;
                break;

            default:
                break;
            }
            break;
        case ir::Type::IntLiteral:
            switch (unaryexp->t)
            {
            case ir::Type::Int:
                switch (term->token.type)
                {
                case TokenType::MULT:
                    instructions.push_back(new ir::Instruction({root->v, root->t},
                                                               {unaryexp->v, unaryexp->t},
                                                               {temp_name, ir::Type::Int},
                                                               ir::Operator::mul));
                    break;
                case TokenType::DIV:
                    instructions.push_back(new ir::Instruction({root->v, root->t},
                                                               {unaryexp->v, unaryexp->t},
                                                               {temp_name, ir::Type::Int},
                                                               ir::Operator::div));
                    break;
                case TokenType::MOD:
                    instructions.push_back(new ir::Instruction({root->v, root->t},
                                                               {unaryexp->v, unaryexp->t},
                                                               {temp_name, ir::Type::Int},
                                                               ir::Operator::mod));
                    break;
                default:
                    break;
                }
                root->v = temp_name;
                root->t = ir::Type::Int;
                break;
            case ir::Type::IntLiteral:
                switch (term->token.type)
                {
                case TokenType::MULT:
                    // instructions.push_back(new ir::Instruction({root->v, root->t},
                    //                                            {unaryexp->v, unaryexp->t},
                    //                                            {temp_name, ir::Type::IntLiteral},
                    //                                            ir::Operator::mul));
                    root->v = std::to_string(std::stoi(root->v) * std::stoi(unaryexp->v));
                    break;
                case TokenType::DIV:
                    // instructions.push_back(new ir::Instruction({root->v, root->t},
                    //                                            {unaryexp->v, unaryexp->t},
                    //                                            {temp_name, ir::Type::IntLiteral},
                    //                                            ir::Operator::div));
                    root->v = std::to_string(std::stoi(root->v) / std::stoi(unaryexp->v));
                    break;
                case TokenType::MOD:
                    // instructions.push_back(new ir::Instruction({root->v, root->t},
                    //                                            {unaryexp->v, unaryexp->t},
                    //                                            {temp_name, ir::Type::IntLiteral},
                    //                                            ir::Operator::mod));
                    root->v = std::to_string(std::stoi(root->v) % std::stoi(unaryexp->v));
                    break;
                default:
                    break;
                }
                root->t = ir::Type::IntLiteral;
                break;

            default:
                break;
            }
            break;
        default:
            break;
        }

        // auto temp_name = get_temp().name;
        // if (term->token.type == TokenType::MULT)
        // {
        //     instructions.push_back(new ir::Instruction({root->v, root->t},
        //                                                {unaryexp->v, unaryexp->t},
        //                                                {temp_name, root->t},
        //                                                ir::Operator::mul));
        // }
        // else if (term->token.type == TokenType::DIV)
        // {
        //     instructions.push_back(new ir::Instruction({root->v, root->t},
        //                                                {unaryexp->v, unaryexp->t},
        //                                                {temp_name, root->t},
        //                                                ir::Operator::div));
        // }
        // else
        // {
        //     instructions.push_back(new ir::Instruction({root->v, root->t},
        //                                                {unaryexp->v, unaryexp->t},
        //                                                {temp_name, root->t},
        //                                                ir::Operator::mod));
        // }
        // root->v = temp_name;
    }
}

// UnaryExp -> PrimaryExp
//           | Ident '(' [FuncRParams] ')'
//           | UnaryOp UnaryExp
// UnaryExp.is_computable
// UnaryExp.v
// UnaryExp.t
void frontend::Analyzer::analysisUnaryExp(UnaryExp *root, std::vector<Instruction *> &instructions)
{
    if (NODE_IS(PRIMARYEXP, 0)) // 如果是PrimaryExp
    {
        GET_CHILD_PTR(primaryexp, PrimaryExp, 0);
        COPY_EXP_NODE(root, primaryexp);
        analysisPrimaryExp(primaryexp, instructions);
        COPY_EXP_NODE(primaryexp, root);
    }
    else if (NODE_IS(TERMINAL, 0)) // 如果是函数调用
    {
        GET_CHILD_PTR(ident, Term, 0);
        ir::Function *func;
        if (symbol_table.functions.count(ident->token.value)) // 如果是用户自定义函数
        {
            func = symbol_table.functions[ident->token.value];
        }
        else if (get_lib_funcs()->count(ident->token.value)) // 如果是库函数
        {
            func = (*get_lib_funcs())[ident->token.value];
        }
        else
        {
            assert(0);
        }
        root->v = get_temp().name;
        root->t = func->returnType;
        if (NODE_IS(FUNCRPARAMS, 2)) // 如果有参数
        {
            GET_CHILD_PTR(funcrparams, FuncRParams, 2);
            vector<ir::Operand> params;
            for (size_t i = 0; i < func->ParameterList.size(); i++)
            {
                params.push_back(get_temp());
                params[i].type = (func->ParameterList)[i].type;
            }
            analysisFuncRParams(funcrparams, params, instructions);

            instructions.push_back(new ir::CallInst({ident->token.value, ir::Type::null}, params, {root->v, root->t}));
        }
        else // 如果没有参数
        {
            instructions.push_back(new ir::CallInst({ident->token.value, ir::Type::null}, {root->v, root->t}));
        }
    }
    else // 如果是一元运算符
    {
        // std::cout << "UnaryOp" << std::endl;
        GET_CHILD_PTR(unaryop, UnaryOp, 0);
        analysisUnaryOp(unaryop, instructions);

        GET_CHILD_PTR(unaryexp, UnaryExp, 1);
        COPY_EXP_NODE(root, unaryexp);
        // std::cout << root->v << std::endl;
        analysisUnaryExp(unaryexp, instructions);
        COPY_EXP_NODE(unaryexp, root);
        // std::cout << root->v << std::endl;

        if (unaryop->op == TokenType::MINU)
        {
            auto temp_name = get_temp().name;
            // std::cout << "UnaryOp: " << toString(root->t) + " " + temp_name << std::endl;
            switch (root->t)
            {
            case ir::Type::Int:
                instructions.push_back(new ir::Instruction({"0", ir::Type::IntLiteral},
                                                           {root->v, root->t},
                                                           {temp_name, ir::Type::Int},
                                                           ir::Operator::sub));
                root->t = ir::Type::Int;
                root->v = temp_name;
                break;
            case ir::Type::IntLiteral:
                // instructions.push_back(new ir::Instruction({"0", ir::Type::IntLiteral},
                //                                            {root->v, root->t},
                //                                            {temp_name, ir::Type::IntLiteral},
                //                                            ir::Operator::sub));
                // root->v = std::to_string(-std::stoi(root->v));

                root->t = ir::Type::IntLiteral;
                // root->v = temp_name;
                // !1!
                root->v = std::to_string(-std::stoi(root->v));
                // std::cout << "UnaryOp: " << toString(root->t) + " " + root->v << std::endl;
                break;
            case ir::Type::Float:
            case ir::Type::FloatLiteral:
                instructions.push_back(new ir::Instruction({"0.0", ir::Type::FloatLiteral},
                                                           {root->v, root->t},
                                                           {temp_name, ir::Type::Float},
                                                           ir::Operator::fsub));
                root->t = ir::Type::Float;
                root->v = temp_name;
                break;
            default:
                assert(0);
            }
        }
        else if (unaryop->op == TokenType::NOT)
        {
            auto temp_name = get_temp().name;
            switch (root->t)
            {
            case ir::Type::Int:
            case ir::Type::IntLiteral:
                instructions.push_back(new ir::Instruction({"0", ir::Type::IntLiteral},
                                                           {root->v, root->t},
                                                           {temp_name, ir::Type::Int},
                                                           ir::Operator::eq));
                root->t = ir::Type::Int;
                break;
            case ir::Type::Float:
            case ir::Type::FloatLiteral:
                instructions.push_back(new ir::Instruction({"0.0", ir::Type::FloatLiteral},
                                                           {root->v, root->t},
                                                           {temp_name, ir::Type::Float},
                                                           ir::Operator::feq));
                root->t = ir::Type::Float;
                break;
            default:
                assert(0);
            }
            root->v = temp_name;
        }
    }
}

// UnaryOp -> '+' | '-' | '!'
// TokenType op;
void frontend::Analyzer::analysisUnaryOp(UnaryOp *root, std::vector<Instruction *> &instructions)
{
    // std::cout << "analysisUnaryOp" << std::endl; // todelete
    GET_CHILD_PTR(term, Term, 0);
    root->op = term->token.type;
}

// FuncRParams -> Exp { ',' Exp }
void frontend::Analyzer::analysisFuncRParams(FuncRParams *root, vector<ir::Operand> &params, vector<ir::Instruction *> &instructions)
{
    size_t index = 0;
    for (size_t i = 0; i < root->children.size(); i += 2)
    {
        if (NODE_IS(EXP, i))
        {
            GET_CHILD_PTR(exp, Exp, i);
            exp->v = params[index].name;
            analysisExp(exp, instructions);
            params[index] = {exp->v, exp->t};
            index++;
        }
    }
}

// PrimaryExp -> '(' Exp ')' | LVal | Number
// PrimaryExp.is_computable
// PrimaryExp.v
// PrimaryExp.t
void frontend::Analyzer::analysisPrimaryExp(PrimaryExp *root, std::vector<Instruction *> &instructions)
{
    if (NODE_IS(NUMBER, 0))
    {
        GET_CHILD_PTR(number, Number, 0);
        COPY_EXP_NODE(root, number);
        analysisNumber(number, instructions);
        COPY_EXP_NODE(number, root);
    }
    else if (NODE_IS(LVAL, 0))
    {
        GET_CHILD_PTR(lval, LVal, 0);
        COPY_EXP_NODE(root, lval);
        analysisLVal(lval, instructions);
        COPY_EXP_NODE(lval, root);
    }
    else
    {
        GET_CHILD_PTR(exp, Exp, 1);
        COPY_EXP_NODE(root, exp);
        analysisExp(exp, instructions);
        COPY_EXP_NODE(exp, root);
    }
}

// LVal -> Ident {'[' Exp ']'}
// LVal.is_computable
// LVal.v
// LVal.t
// LVal.i array index, legal if t is IntPtr or FloatPtr
void frontend::Analyzer::analysisLVal(LVal *root, vector<ir::Instruction *> &instructions)
{
    GET_CHILD_PTR(ident, Term, 0);
    auto var = symbol_table.get_ste(ident->token.value);

    root->i = 0;

    // std::cout << "LVal: " << toString(var.operand.type) << " "
    //           << var.operand.name << " "
    //           << "[" << var.dimension.size() << "]" << std::endl;

    if (root->children.size() == 1) // 如果没有下标
    {
        switch (var.operand.type)
        {
        case ir::Type::IntPtr:
        case ir::Type::FloatPtr:
            std::cout << "! To Figure out Why" << std::endl;
            root->is_computable = false;
            root->t = var.operand.type;
            root->v = var.operand.name;
            break;
        case ir::Type::IntLiteral:
        case ir::Type::FloatLiteral:
            root->is_computable = true;
            root->t = var.operand.type;
            root->v = var.operand.name;
            break;
        default:
            root->t = var.operand.type;
            root->v = var.operand.name;
            break;
        }
    }
    else // 如果有下标
    {
        root->i = 1;
        std::vector<ir::Operand> load_index;
        for (size_t index = 2; index < root->children.size(); index += 3)
        {
            if (!(NODE_IS(EXP, index)))
                break;

            GET_CHILD_PTR(exp, Exp, index);
            // exp->v = get_temp().name;
            analysisExp(exp, instructions);
            std::cout << "index: " << toString(exp->t) << " " << exp->v << std::endl; //! 4
            // auto op = symbol_table.get_operand(exp->v);
            // std::cout << toString(op.type) << " " << op.name << std::endl;
            load_index.push_back({exp->v, exp->t});
        }

        auto res_index = get_temp(ir::Type::Int);
        instructions.push_back(new ir::Instruction({"0", ir::Type::IntLiteral},
                                                   {},
                                                   res_index,
                                                   ir::Operator::def));
        for (size_t i = 0; i < load_index.size(); i++)
        {
            int mul_dim = std::accumulate(var.dimension.begin() + i + 1, var.dimension.end(), 1, std::multiplies<int>());
            auto temp_name = get_temp().name;
            instructions.push_back(new ir::Instruction(load_index[i],
                                                       {std::to_string(mul_dim), ir::Type::IntLiteral},
                                                       {temp_name, ir::Type::Int},
                                                       ir::Operator::mul));
            instructions.push_back(new ir::Instruction(res_index,
                                                       {temp_name, ir::Type::Int},
                                                       res_index,
                                                       ir::Operator::add));
        }

        // for (size_t i = 0; i < load_index.size(); i++)
        // {
        //     root->i += load_index[i] * std::accumulate(var.dimension.begin() + i + 1, var.dimension.end(), 1, std::multiplies<int>());
        // }
        // std::cout << "root->i: " << cal_index << std::endl;

        // 取数指令，这里load指从数组中取数。第一个操作数为数组名，第二个操作数为要取数所在数组下标，目的操作数为取数存放变量。示例如下：
        //      a = arr[2]; => load a, arr, 2
        //      ir::Operand temp = get_temp();
        instructions.push_back(new ir::Instruction(var.operand,
                                                   res_index,
                                                   {root->v, root->t},
                                                   ir::Operator::load));
    }
}

// Number -> IntConst | floatConst
// Number.is_computable = true;
// Number.v
// Number.t
void frontend::Analyzer::analysisNumber(Number *root, vector<ir::Instruction *> &)
{
    root->is_computable = true;

    GET_CHILD_PTR(term, Term, 0);

    if (term->token.value.substr(0, 2) == "0x")
    {
        root->v = std::to_string(std::stoi(term->token.value, nullptr, 16));
    }
    else if (term->token.value.substr(0, 2) == "0b")
    {
        root->v = std::to_string(std::stoi(term->token.value, nullptr, 2));
    }
    else if (term->token.value.substr(0, 1) == "0" && !(term->token.value.substr(0, 2) == "0x") && !(term->token.value.substr(0, 2) == "0b"))
    {
        root->v = std::to_string(std::stoi(term->token.value, nullptr, 8));
    }
    else
    {
        root->v = term->token.value;
    }

    switch (term->token.type)
    {
    case TokenType::INTLTR: // 整数常量
        root->t = Type::IntLiteral;
        break;
    case TokenType::FLOATLTR: // 浮点数常量
        root->t = Type::FloatLiteral;
        break;
    default:
        break;
    }
}
