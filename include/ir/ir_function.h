#ifndef IRFUNCTION_H
#define IRFUNCTION_H
#include <vector>
#include <string>
#include "ir/ir_operand.h"
#include "ir/ir_instruction.h"
namespace ir
{

struct Function {
    std::string name; // 函数块名称，可以直接将源程序中函数名作为name
    ir::Type returnType; // 函数返回类型，即对应源程序中函数的返回类型。
    std::vector<Operand> ParameterList;
    std::vector<Instruction*> InstVec;
    Function();
    Function(const std::string&, const ir::Type&);
    Function(const std::string&, const std::vector<Operand>&, const ir::Type&);
    void addInst(Instruction* inst);
    std::string draw();
};

}
#endif
