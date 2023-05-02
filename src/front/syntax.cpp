#include "front/syntax.h"

#include <iostream>
#include <cassert>

using frontend::AddExp;
using frontend::AstNode;
using frontend::Block;
using frontend::BlockItem;
using frontend::BType;
using frontend::CompUnit;
using frontend::Cond;
using frontend::ConstDecl;
using frontend::ConstDef;
using frontend::ConstExp;
using frontend::ConstInitVal;
using frontend::Decl;
using frontend::EqExp;
using frontend::Exp;
using frontend::FuncDef;
using frontend::FuncFParam;
using frontend::FuncFParams;
using frontend::FuncRParams;
using frontend::FuncType;
using frontend::InitVal;
using frontend::LAndExp;
using frontend::LOrExp;
using frontend::LVal;
using frontend::MulExp;
using frontend::Number;
using frontend::Parser;
using frontend::PrimaryExp;
using frontend::RelExp;
using frontend::Stmt;
using frontend::Term;
using frontend::UnaryExp;
using frontend::UnaryOp;
using frontend::VarDecl;
using frontend::VarDef;

#define DEBUG_PARSER
#define TODO assert(0 && "todo")
#define CUR_TOKEN_IS(tk_type) (token_stream[index].type == TokenType::tk_type)
#define PARSE_TOKEN(tk_type) root->children.push_back(parseTerm(root, TokenType::tk_type))
// name是要创建的AST节点的名称，type是要解析的语法元素的类型。
#define PARSE(name, type)       \
    auto name = new type(root); \
    assert(parse##type(name));  \
    root->children.push_back(name);

Parser::Parser(const std::vector<frontend::Token> &tokens) : index(0), token_stream(tokens)
{
}

Parser::~Parser() {}

CompUnit *Parser::get_abstract_syntax_tree()
{
    CompUnit *root = new CompUnit();

    parseCompUnit(root);

    return root;
}

Term *Parser::parseTerm(AstNode *parent, TokenType expected)
{
    if (token_stream[index].type == expected)
    {
        std::cout << "TERM: " << toString(parent->type) << "\t" << token_stream[index].value << '\n';
        Term *node = new Term(token_stream[index], parent);
        index++;
        return node;
    }
}

// CompUnit -> (Decl | FuncDef) [CompUnit]
bool Parser::parseCompUnit(CompUnit *root)
{

    if (CUR_TOKEN_IS(CONSTTK))
    {
        PARSE(decl, Decl);
    }
    else if (CUR_TOKEN_IS(VOIDTK))
    {
        PARSE(func_def, FuncDef);
    }
    else if (CUR_TOKEN_IS(INTTK) || CUR_TOKEN_IS(FLOATTK))
    {

        if (index < token_stream.size() - 2 && token_stream[index + 2].type == TokenType::LPARENT)
        {
            PARSE(func_def, FuncDef);
        }
        else
        {
            PARSE(decl, Decl);
        }
    }

    if (CUR_TOKEN_IS(CONSTTK) || CUR_TOKEN_IS(VOIDTK) || CUR_TOKEN_IS(INTTK) || CUR_TOKEN_IS(FLOATTK))
    {
        PARSE(comp_unit, CompUnit);
    }

    return true;
}

// Decl -> ConstDecl | VarDecl
bool Parser::parseDecl(Decl *root)
{

    if (CUR_TOKEN_IS(CONSTTK))
    {
        PARSE(const_decl, ConstDecl);
    }
    else if (CUR_TOKEN_IS(INTTK) || CUR_TOKEN_IS(FLOATTK))
    {
        PARSE(var_decl, VarDecl);
    }
    else
    {
        return false;
    }

    return true;
}

// ConstDecl -> 'const' BType ConstDef { ',' ConstDef } ';'
bool Parser::parseConstDecl(ConstDecl *root)
{
    PARSE_TOKEN(CONSTTK);
    PARSE(b_typr, BType);
    PARSE(const_def, ConstDef);

    while (CUR_TOKEN_IS(COMMA))
    {
        PARSE_TOKEN(COMMA);
        PARSE(const_def, ConstDef);
    }

    PARSE_TOKEN(SEMICN);

    return true;
}

// BType -> 'int' | 'float'
bool Parser::parseBType(BType *root)
{

    if (CUR_TOKEN_IS(INTTK))
    {
        PARSE_TOKEN(INTTK);
    }
    else if (CUR_TOKEN_IS(FLOATTK))
    {
        PARSE_TOKEN(FLOATTK);
    }

    return true;
}

// ConstDef -> Ident { '[' ConstExp ']' } '=' ConstInitVal
bool Parser::parseConstDef(ConstDef *root)
{

    PARSE_TOKEN(IDENFR);

    while (CUR_TOKEN_IS(LBRACK))
    {
        PARSE_TOKEN(LBRACK);
        PARSE(const_exp, ConstExp);
        PARSE_TOKEN(RBRACK);
    }

    PARSE_TOKEN(ASSIGN);
    PARSE(const_init_val, ConstInitVal);

    return true;
}

// ConstInitVal -> ConstExp | '{' [ ConstInitVal { ',' ConstInitVal } ] '}'
bool Parser::parseConstInitVal(ConstInitVal *root)
{

    // ConstInitVal -> '{' [ ConstInitVal { ',' ConstInitVal } ] '}'
    if (CUR_TOKEN_IS(LBRACE))
    {
        PARSE_TOKEN(LBRACE);

        if (!CUR_TOKEN_IS(RBRACE))
        {
            PARSE(const_init_val, ConstInitVal);
            while (CUR_TOKEN_IS(COMMA))
            {
                PARSE_TOKEN(COMMA);
                PARSE(const_init_val, ConstInitVal);
            }
        }

        PARSE_TOKEN(RBRACE);
    }
    // ConstInitVal -> ConstExp
    else
    {
        PARSE(const_exp, ConstExp);
    }

    return true;
}

// VarDecl -> BType VarDef { ',' VarDef } ';'
bool Parser::parseVarDecl(VarDecl *root)
{

    PARSE(b_type, BType);

    PARSE(var_def, VarDef);

    while (CUR_TOKEN_IS(COMMA))
    {
        PARSE_TOKEN(COMMA);
        PARSE(var_def, VarDef);
    }

    PARSE_TOKEN(SEMICN);

    return true;
}

// VarDef -> Ident { '[' ConstExp ']' } [ '=' InitVal ]
bool Parser::parseVarDef(VarDef *root)
{

    PARSE_TOKEN(IDENFR);

    while (CUR_TOKEN_IS(LBRACK))
    {
        PARSE_TOKEN(LBRACK);
        PARSE(const_exp, ConstExp);
        PARSE_TOKEN(RBRACK);
    }

    if (CUR_TOKEN_IS(ASSIGN))
    {
        PARSE_TOKEN(ASSIGN);
        PARSE(init_val, InitVal);
    }

    return true;
}

// InitVal -> Exp | '{' [ InitVal { ',' InitVal } ] '}'
bool Parser::parseInitVal(InitVal *root)
{

    // InitVal -> '{' [ InitVal { ',' InitVal } ] '}'
    if (CUR_TOKEN_IS(LBRACE))
    {
        PARSE_TOKEN(LBRACE);
        // InitVal -> '{' '}'
        if (!CUR_TOKEN_IS(RBRACE))
        {
            PARSE(init_val, InitVal);
            while (CUR_TOKEN_IS(COMMA))
            {
                PARSE_TOKEN(COMMA);
                PARSE(init_val, InitVal);
            }
        }

        PARSE_TOKEN(RBRACE);
    }
    // InitVal -> Exp
    else
    {
        PARSE(exp, Exp);
    }

    return true;
}

// FuncDef -> FuncType Ident '(' [FuncFParams] ')' Block
bool Parser::parseFuncDef(FuncDef *root)
{

    PARSE(func_type, FuncType);
    PARSE_TOKEN(IDENFR);
    PARSE_TOKEN(LPARENT);

    // no [FuncFParams], FuncType Ident '(' ')' Block
    if (CUR_TOKEN_IS(RPARENT))
    {
        PARSE_TOKEN(RPARENT);
    }
    // FuncType Ident '(' FuncFParams ')' Block
    else
    {
        PARSE(func_params, FuncFParams);
        PARSE_TOKEN(RPARENT);
    }
    PARSE(block, Block);

    return true;
}

// FuncType -> 'void' | 'int' | 'float'
bool Parser::parseFuncType(FuncType *root)
{

    if (CUR_TOKEN_IS(VOIDTK))
    {
        PARSE_TOKEN(VOIDTK);
    }
    else if (CUR_TOKEN_IS(INTTK))
    {
        PARSE_TOKEN(INTTK);
    }
    else
    {
        PARSE_TOKEN(FLOATTK);
    }

    return true;
}

// FuncFParam -> BType Ident ['[' ']' { '[' Exp ']' }]
bool Parser::parseFuncFParam(FuncFParam *root)
{

    PARSE(b_type, BType);

    PARSE_TOKEN(IDENFR);

    // FuncFParam -> BType Ident '[' ']' { '[' Exp ']' }
    if (CUR_TOKEN_IS(LBRACK))
    {
        PARSE_TOKEN(LBRACK);
        PARSE_TOKEN(RBRACK);
        while (CUR_TOKEN_IS(LBRACK))
        {
            PARSE_TOKEN(LBRACK);
            PARSE(exp, Exp);
            PARSE_TOKEN(RBRACK);
        }
    }

    return true;
}

// FuncFParams -> FuncFParam { ',' FuncFParam }
bool Parser::parseFuncFParams(FuncFParams *root)
{

    PARSE(func_pram, FuncFParam);

    while (CUR_TOKEN_IS(COMMA))
    {
        PARSE_TOKEN(COMMA);
        PARSE(func_f_pram, FuncFParam);
    }

    return true;
}

// Block -> '{' { BlockItem } '}'
bool Parser::parseBlock(Block *root)
{

    PARSE_TOKEN(LBRACE);

    // BlockItem exist
    while (!CUR_TOKEN_IS(RBRACE))
    {
        PARSE(block_item, BlockItem);
    }
    PARSE_TOKEN(RBRACE);

    return true;
}

// BlockItem -> Decl | Stmt
bool Parser::parseBlockItem(BlockItem *root)
{

    if (CUR_TOKEN_IS(CONSTTK) || CUR_TOKEN_IS(VOIDTK) || CUR_TOKEN_IS(INTTK) || CUR_TOKEN_IS(FLOATTK))
    {
        PARSE(decl, Decl);
    }
    else
    {
        PARSE(stmt, Stmt);
    }

    return true;
}

// Stmt -> LVal '=' Exp ';'                         first = { Ident }
//       | Block                                    first = { '{' }
//       | 'if' '(' Cond ')' Stmt [ 'else' Stmt ]   first = { 'if' }
//       | 'while' '(' Cond ')' Stmt                first = { 'while' }
//       | 'break' ';'                              first = { 'break' }
//       | 'continue' ';'                           first = { 'continue' }
//       | 'return' [Exp] ';'                       first = { 'return' }
//       | [Exp] ';'                                first = { Exp / ';' }
bool Parser::parseStmt(Stmt *root)
{

    // Stmt -> Block
    if (CUR_TOKEN_IS(LBRACE))
    {
        PARSE(block, Block);
    }
    // Stmt -> 'if' '(' Cond ')' Stmt [ 'else' Stmt ]
    else if (CUR_TOKEN_IS(IFTK))
    {
        PARSE_TOKEN(IFTK);
        PARSE_TOKEN(LPARENT);
        PARSE(cond, Cond);
        PARSE_TOKEN(RPARENT);
        PARSE(stmt, Stmt);
        if (CUR_TOKEN_IS(ELSETK))
        {
            PARSE_TOKEN(ELSETK);
            PARSE(stmt, Stmt);
        }
    }
    // Stmt -> 'while' '(' Cond ')' Stmt
    else if (CUR_TOKEN_IS(WHILETK))
    {
        PARSE_TOKEN(WHILETK);
        PARSE_TOKEN(LPARENT);
        PARSE(cond, Cond);
        PARSE_TOKEN(RPARENT);
        PARSE(stmt, Stmt);
    }
    // Stmt -> 'break' ';'
    else if (CUR_TOKEN_IS(BREAKTK))
    {
        PARSE_TOKEN(BREAKTK);
        PARSE_TOKEN(SEMICN);
    }
    // Stmt -> 'continue' ';'
    else if (CUR_TOKEN_IS(CONTINUETK))
    {
        PARSE_TOKEN(CONTINUETK);
        PARSE_TOKEN(SEMICN);
    }
    // Stmt -> 'return' [Exp] ';'
    else if (CUR_TOKEN_IS(RETURNTK))
    {
        PARSE_TOKEN(RETURNTK);
        if (!CUR_TOKEN_IS(SEMICN))
        {
            PARSE(exp, Exp);
        }
        PARSE_TOKEN(SEMICN);
    }
    // Stmt -> ';'
    else if (CUR_TOKEN_IS(SEMICN))
    {
        PARSE_TOKEN(SEMICN);
    }
    // Stmt -> Exp ';'
    else if (CUR_TOKEN_IS(LPARENT) || CUR_TOKEN_IS(INTTK) || CUR_TOKEN_IS(FLOATTK) || CUR_TOKEN_IS(PLUS) || CUR_TOKEN_IS(MINU) || CUR_TOKEN_IS(NOT))
    {
        PARSE(exp, Exp);
        PARSE_TOKEN(SEMICN);
    }
    // Stmt -> LVal '=' Exp ';' | Exp ';'
    // TODO
    else if (CUR_TOKEN_IS(IDENFR))
    {
        // Stmt -> Exp ';'
        //      -> Ident '(' [FuncRParams] ')'
        if (index < token_stream.size() - 1 && token_stream[index + 1].type == TokenType::LPARENT)
        {
            PARSE(exp, Exp);
            PARSE_TOKEN(SEMICN);
        }
        // Stmt -> LVal '=' Exp ';'
        //      -> Ident {'[' Exp ']'}
        else if (index < token_stream.size() - 1 && (token_stream[index + 1].type == TokenType::LBRACK) || (token_stream[index + 1].type == TokenType::ASSIGN))
        {
            PARSE(l_val, LVal);
            PARSE_TOKEN(ASSIGN);
            PARSE(exp, Exp);
            PARSE_TOKEN(SEMICN);
        }
    }

    return true;
}

// Exp -> AddExp
bool Parser::parseExp(Exp *root)
{

    PARSE(add_exp, AddExp);

    return true;
}

// Cond -> LOrExp
bool Parser::parseCond(Cond *root)
{

    PARSE(lor_exp, LOrExp);

    return true;
}

// LVal -> Ident {'[' Exp ']'}
bool Parser::parseLVal(LVal *root)
{

    PARSE_TOKEN(IDENFR);

    while (CUR_TOKEN_IS(LBRACK))
    {
        PARSE_TOKEN(LBRACK);
        PARSE(exp, Exp);
        PARSE_TOKEN(RBRACK);
    }

    return true;
}

// Number -> IntConst | floatConst
bool Parser::parseNumber(Number *root)
{

    if (CUR_TOKEN_IS(INTLTR))
    {
        PARSE_TOKEN(INTLTR);
    }
    else
    {
        PARSE_TOKEN(FLOATLTR);
    }

    return true;
}

// PrimaryExp -> '(' Exp ')' | LVal | Number
bool Parser::parsePrimaryExp(PrimaryExp *root)
{

    if (CUR_TOKEN_IS(LPARENT))
    {
        PARSE_TOKEN(LPARENT);
        PARSE(exp, Exp);
        PARSE_TOKEN(RPARENT);
    }
    else if (CUR_TOKEN_IS(IDENFR))
    {
        PARSE(lval, LVal);
    }
    else
    {
        PARSE(number, Number);
    }

    return true;
}

// UnaryExp -> PrimaryExp                   first = { '(' , Ident, int, float }
//          | Ident '(' [FuncRParams] ')'   first = { Ident }
//          | UnaryOp UnaryExp              first = { '+' , '-' , '!' }
bool Parser::parseUnaryExp(UnaryExp *root)
{

    // UnaryExp -> UnaryOp UnaryExp
    if (CUR_TOKEN_IS(PLUS) || CUR_TOKEN_IS(MINU) || CUR_TOKEN_IS(NOT))
    {
        PARSE(unary_op, UnaryOp);
        PARSE(unary_exp, UnaryExp);
    }
    // UnaryExp -> PrimaryExp
    else if (CUR_TOKEN_IS(LPARENT) || CUR_TOKEN_IS(INTLTR) || CUR_TOKEN_IS(FLOATLTR))
    {
        PARSE(promary_exp, PrimaryExp);
    }
    else if (CUR_TOKEN_IS(IDENFR))
    {
        // UnaryExp -> Ident '(' [FuncRParams] ')'
        if (index < token_stream.size() - 1 && token_stream[index + 1].type == TokenType::LPARENT)
        {
            PARSE_TOKEN(IDENFR);
            PARSE_TOKEN(LPARENT);
            if (!CUR_TOKEN_IS(RPARENT))
            {
                PARSE(func_r_params, FuncRParams);
            }
            PARSE_TOKEN(RPARENT);
        }
        // UnaryExp -> PrimaryExp
        else
        {
            PARSE(promary_exp, PrimaryExp);
        }
    }

    return true;
}

// UnaryOp -> '+' | '-' | '!'
bool Parser::parseUnaryOp(UnaryOp *root)
{

    if (CUR_TOKEN_IS(PLUS))
    {
        PARSE_TOKEN(PLUS);
    }
    else if (CUR_TOKEN_IS(MINU))
    {
        PARSE_TOKEN(MINU);
    }
    else
    {
        PARSE_TOKEN(NOT);
    }

    return true;
}

// FuncRParams -> Exp { ',' Exp }
bool Parser::parseFuncRParams(FuncRParams *root)
{

    PARSE(exp, Exp);
    while (CUR_TOKEN_IS(COMMA))
    {
        PARSE_TOKEN(COMMA);
        PARSE(exp, Exp);
    }

    return true;
}

// MulExp -> UnaryExp { ('*' | '/' | '%') UnaryExp }
bool Parser::parseMulExp(MulExp *root)
{

    PARSE(unary_exp, UnaryExp);

    while (CUR_TOKEN_IS(MULT) || CUR_TOKEN_IS(DIV) || CUR_TOKEN_IS(MOD))
    {
        if (CUR_TOKEN_IS(MULT))
        {
            PARSE_TOKEN(MULT);
        }
        else if (CUR_TOKEN_IS(DIV))
        {
            PARSE_TOKEN(DIV);
        }
        else
        {
            PARSE_TOKEN(MOD);
        }
        PARSE(unary_exp, UnaryExp);
    }

    return true;
}

// AddExp -> MulExp { ('+' | '-') MulExp }
bool Parser::parseAddExp(AddExp *root)
{

    PARSE(mul_exp, MulExp);

    while (CUR_TOKEN_IS(PLUS) || CUR_TOKEN_IS(MINU))
    {
        if (CUR_TOKEN_IS(PLUS))
        {
            PARSE_TOKEN(PLUS);
        }
        else if (CUR_TOKEN_IS(MINU))
        {
            PARSE_TOKEN(MINU);
        }
        else
        {
            return false;
        }
        PARSE(mul_exp, MulExp);
    }

    return true;
}

// RelExp -> AddExp { ('<' | '>' | '<=' | '>=') AddExp }
bool Parser::parseRelExp(RelExp *root)
{

    PARSE(add_exp, AddExp);

    while (CUR_TOKEN_IS(LSS) || CUR_TOKEN_IS(GTR) || CUR_TOKEN_IS(LEQ) || CUR_TOKEN_IS(GEQ))
    {
        if (CUR_TOKEN_IS(LSS))
        {
            PARSE_TOKEN(LSS);
        }
        else if (CUR_TOKEN_IS(GTR))
        {
            PARSE_TOKEN(GTR);
        }
        else if (CUR_TOKEN_IS(LEQ))
        {
            PARSE_TOKEN(LEQ);
        }
        else if (CUR_TOKEN_IS(GEQ))
        {
            PARSE_TOKEN(GEQ);
        }
        else
        {
            return false;
        }
        PARSE(add_exp, AddExp);
    }

    return true;
}

// EqExp -> RelExp { ('==' | '!=') RelExp }
bool Parser::parseEqExp(EqExp *root)
{

    PARSE(rel_exp, RelExp);

    while (CUR_TOKEN_IS(EQL) || CUR_TOKEN_IS(NEQ))
    {
        if (CUR_TOKEN_IS(EQL))
        {
            PARSE_TOKEN(EQL);
        }
        else if (CUR_TOKEN_IS(NEQ))
        {
            PARSE_TOKEN(NEQ);
        }
        else
        {
            return false;
        }
        PARSE(rel_exp, RelExp);
    }

    return true;
}

// LAndExp -> EqExp [ '&&' LAndExp ]
bool Parser::parseLAndExp(LAndExp *root)
{

    PARSE(eq_exp, EqExp);

    if (CUR_TOKEN_IS(AND))
    {
        PARSE_TOKEN(AND);
        PARSE(l_and_exp, LAndExp);
    }

    return true;
}

// LOrExp -> LAndExp [ '||' LOrExp ]
bool Parser::parseLOrExp(LOrExp *root)
{

    PARSE(l_and_exp, LAndExp);

    if (CUR_TOKEN_IS(OR))
    {
        PARSE_TOKEN(OR);
        PARSE(l_or_exp, LOrExp);
    }

    return true;
}

// ConstExp -> AddExp
bool Parser::parseConstExp(ConstExp *root)
{

    PARSE(add_exp, AddExp);

    return true;
}

void Parser::log(AstNode *node)
{
#ifdef DEBUG_PARSER
    std::cout << "in parse" << toString(node->type) << ", cur_token_type::" << toString(token_stream[index].type) << ", token_val::" << token_stream[index].value << '\n';
#endif
}
