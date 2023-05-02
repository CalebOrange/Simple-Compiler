#include "front/lexical.h"

#include <map>
#include <cassert>
#include <string>

#define TODO assert(0 && "todo")

// #define DEBUG_DFA_BEGIN
// #define DEBUG_DFA_END
// #define DEBUG_SCANNER

std::string frontend::toString(State s)
{
    switch (s)
    {
    case State::Empty:
        return "Empty";
    case State::Ident:
        return "Ident";
    case State::IntLiteral:
        return "IntLiteral";
    case State::FloatLiteral:
        return "FloatLiteral";
    case State::op:
        return "op";
    default:
        assert(0 && "invalid State");
    }
    return "";
}

std::set<std::string> frontend::keywords = {
    "const", "int", "float", "if", "else", "while", "continue", "break", "return", "void"};

frontend::DFA::DFA() : cur_state(frontend::State::Empty), cur_str() {}

frontend::DFA::~DFA() {}

frontend::TokenType get_keywords_type(std::string s)
{
    // std::cout << "get_keywords_type:" << s << std::endl;
    if (s == "const")
        return frontend::TokenType::CONSTTK;
    else if (s == "int")
        return frontend::TokenType::INTTK;
    else if (s == "float")
        return frontend::TokenType::FLOATTK;
    else if (s == "if")
        return frontend::TokenType::IFTK;
    else if (s == "else")
        return frontend::TokenType::ELSETK;
    else if (s == "while")
        return frontend::TokenType::WHILETK;
    else if (s == "continue")
        return frontend::TokenType::CONTINUETK;
    else if (s == "break")
        return frontend::TokenType::BREAKTK;
    else if (s == "return")
        return frontend::TokenType::RETURNTK;
    else if (s == "void")
        return frontend::TokenType::VOIDTK;
    else
        return frontend::TokenType::IDENFR;
}

frontend::TokenType get_op_type(std::string s)
{
    if (s.size() == 1)
    {
        switch (s[0])
        {
        case '+':
            return frontend::TokenType::PLUS;
        case '-':
            return frontend::TokenType::MINU;
        case '*':
            return frontend::TokenType::MULT;
        case '/':
            return frontend::TokenType::DIV;
        case '%':
            return frontend::TokenType::MOD;
        case '<':
            return frontend::TokenType::LSS;
        case '>':
            return frontend::TokenType::GTR;
        case ':':
            return frontend::TokenType::COLON;
        case '=':
            return frontend::TokenType::ASSIGN;
        case ';':
            return frontend::TokenType::SEMICN;
        case ',':
            return frontend::TokenType::COMMA;
        case '(':
            return frontend::TokenType::LPARENT;
        case ')':
            return frontend::TokenType::RPARENT;
        case '[':
            return frontend::TokenType::LBRACK;
        case ']':
            return frontend::TokenType::RBRACK;
        case '{':
            return frontend::TokenType::LBRACE;
        case '}':
            return frontend::TokenType::RBRACE;
        case '!':
            return frontend::TokenType::NOT;
        default:
            assert(0 && "invalid op type");
        }
    }
    else if (s.size() == 2)
    {
        if (s == "<=")
            return frontend::TokenType::LEQ;
        else if (s == ">=")
            return frontend::TokenType::GEQ;
        else if (s == "==")
            return frontend::TokenType::EQL;
        else if (s == "!=")
            return frontend::TokenType::NEQ;
        else if (s == "&&")
            return frontend::TokenType::AND;
        else if (s == "||")
            return frontend::TokenType::OR;
        else
            assert(0 && "invalid op type");
    }
}

bool is_letter(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool is_digit(char c)
{
    return c >= '0' && c <= '9';
}

bool is_dot(char c)
{
    return c == '.';
}

bool is_single_operator(std::string str)
{
    return (
        str == "+" ||
        str == "-" ||
        str == "*" ||
        str == "/" ||
        str == "%" ||
        str == ":" ||
        str == ";" ||
        str == "," ||
        str == "(" ||
        str == ")" ||
        str == "[" ||
        str == "]" ||
        str == "{" ||
        str == "}");
}

bool is_prefix_operator(char c)
{
    return (
        c == '<' ||
        c == '>' ||
        c == '=' ||
        c == '!' ||
        c == '&' ||
        c == '|');
}

bool is_compound_operator(std::string str)
{
    return (
        str == "<=" ||
        str == ">=" ||
        str == "==" ||
        str == "!=" ||
        str == "&&" ||
        str == "||");
}

bool is_operator(char c)
{
    return (
        c == '+' ||
        c == '-' ||
        c == '*' ||
        c == '/' ||
        c == '%' ||
        c == ':' ||
        c == ';' ||
        c == ',' ||
        c == '(' ||
        c == ')' ||
        c == '[' ||
        c == ']' ||
        c == '{' ||
        c == '}' ||
        c == '<' ||
        c == '>' ||
        c == '=' ||
        c == '!' ||
        c == '&' ||
        c == '|');
}

bool is_ident_composition(char c)
{
    return is_letter(c) || c == '_';
}

bool frontend::DFA::next(char input, Token &buf)
{
#ifdef DEBUG_DFA_BEGIN
#include <iostream>
    std::cout << "in state [" << toString(cur_state) << "], input = \'" << input << "\', str = " << cur_str << "\t";
#endif
    bool is_token = false; // 当前字符串是否为token
    if (cur_state == State::Empty)
    {
        // 状态转移
        if (is_ident_composition(input))
            cur_state = State::Ident;
        else if (is_digit(input))
            cur_state = State::IntLiteral;
        else if (is_dot(input))
            cur_state = State::FloatLiteral;
        else if (is_operator(input))
            cur_state = State::op;
        else
        {
            reset();
            return is_token;
        }
        cur_str += input;
    }
    else if (cur_state == State::Ident)
    {
        // 状态转移
        if (is_operator(input))
        {
            cur_state = State::op;
            is_token = true;
        }
        else if (is_digit(input))
        {
            cur_state = State::Ident;
        }
        else if (is_ident_composition(input))
        {
            cur_state = State::Ident;
        }
        else
        {
            is_token = true;
            cur_state = State::Empty;
        }

        // 更新字符串
        if (is_token)
        {
            buf.value = cur_str;
            buf.type = get_keywords_type(cur_str);
            // std::cout << "get_keywords_type:" << cur_str << std::endl;
            cur_str = "";
        }
        if (cur_state != State::Empty)
            cur_str += input;
    }
    else if (cur_state == State::op)
    {
        // 状态转移
        if (is_digit(input))
        {
            cur_state = State::IntLiteral;
            is_token = true;
        }
        else if (is_dot(input)){
            cur_state = State::FloatLiteral;
            is_token = true;
        }
        else if (is_ident_composition(input))
        {
            cur_state = State::Ident;
            is_token = true;
        }
        else if (is_operator(input))
        {
            if (is_single_operator(cur_str) || is_compound_operator(cur_str) || !is_compound_operator(cur_str + input))
            {
                is_token = true;
            }
        }
        else
        {
            is_token = true;
            cur_state = State::Empty;
        }

        // 更新字符串
        if (is_token)
        {
            buf.value = cur_str;
            buf.type = get_op_type(cur_str);
            cur_str = "";
        }
        if (cur_state != State::Empty)
            cur_str += input;
    }
    else if (cur_state == State::IntLiteral)
    {
        // 状态转移
        if (is_dot(input))
            cur_state = State::FloatLiteral;
        else if (is_letter(input))
        {
            cur_state = State::IntLiteral;
        }
        else if (is_operator(input))
        {
            cur_state = State::op;
            is_token = true;
        }
        else if (is_digit(input))
        {
            cur_state = State::IntLiteral;
        }
        else
        {
            is_token = true;
            cur_state = State::Empty;
        }

        // 更新字符串
        if (is_token)
        {
            buf.value = cur_str;
            buf.type = TokenType::INTLTR;
            cur_str = "";
        }
        if (cur_state != State::Empty)
            cur_str += input;
    }
    else if (cur_state == State::FloatLiteral)
    {
        // 状态转移
        if (is_letter(input))
        {
            cur_state = State::FloatLiteral;
        }
        else if (is_operator(input))
        {
            cur_state = State::op;
            is_token = true;
        }
        else if (is_digit(input))
        {
            cur_state = State::FloatLiteral;
        }
        else
        {
            is_token = true;
            cur_state = State::Empty;
        }

        // 更新字符串
        if (is_token)
        {
            buf.value = cur_str;
            buf.type = TokenType::FLOATLTR;
            cur_str = "";
        }
        if (cur_state != State::Empty)
            cur_str += input;
    }

#ifdef DEBUG_DFA_END
    std::cout << ", next state is [" << toString(cur_state) << "], next str = " << cur_str << std::endl;
#endif

    return is_token;
}

void frontend::DFA::reset()
{
    cur_state = State::Empty;
    cur_str = "";
}

frontend::Scanner::Scanner(std::string filename) : fin(filename)
{
    if (!fin.is_open())
    {
        assert(0 && "in Scanner constructor, input file cannot open");
    }
}

frontend::Scanner::~Scanner()
{
    fin.close();
}

std::vector<frontend::Token> frontend::Scanner::run()
{
    std::string line;
    std::vector<frontend::Token> tk_steam;
    bool comment_tag = false;
    frontend::Token tk;
    frontend::DFA dfa;
    while (std::getline(fin, line))
    {
        line += '\n';
        // 删除字符串开头的空格
        line.erase(0, line.find_first_not_of(" "));
        for (size_t i = 0; i < line.size(); i++)
        {
            if (i < line.size() - 1 && line[i] == '/' && line[i + 1] == '/')
            {
                break;
            }
            if (i < line.size() - 1 && line[i] == '/' && line[i + 1] == '*')
            {
                comment_tag = true;
                i += 2;
            }
            if (i < line.size() - 1 && line[i] == '*' && line[i + 1] == '/')
            {
                comment_tag = false;
                i += 2;
            }

            if (!comment_tag && dfa.next(line[i], tk))
            {
                tk_steam.push_back(tk);
#ifdef DEBUG_SCANNER
#include <iostream>
                std::cout << "token: " << toString(tk.type) << "\t" << tk.value << std::endl;
#endif
            }
        }
    }

    return tk_steam;
}