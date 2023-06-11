#include "backend/generator.h"

#include <assert.h>

#define TODO assert(0 && "todo")

backend::Generator::Generator(ir::Program &p, std::ofstream &f) : program(p), fout(f) {}

void backend::Generator::gen()
{
    for (auto &global_val : program.globalVal)
    {
        fout << ".global " << global_val.first << std::endl;
        fout << ".align 2" << std::endl;
        fout << ".type " << global_val.first << ", @object" << std::endl;
        fout << ".size " << global_val.first << ", 4" << std::endl;
        fout << global_val.first << ":" << std::endl;
        fout << ".word " << global_val.second << std::endl;
    }
    for (auto &func : program.functions)
    {
        gen_func(func);
    }
}

void backend::Generator::gen_func(const ir::Function &){}
void backend::Generator::gen_instr(const ir::Instruction &){}
