#include "backend/generator.h"

#include <assert.h>

#define TODO assert(0 && "todo")

backend::Generator::Generator(ir::Program &p, std::ofstream &f) : program(p), fout(f) {}

void backend::Generator::gen()
{
    for (auto &global_val : program.globalVal)
    {
        fout << "\t.global " << global_val.val.name << std::endl;
        fout << "\t.align 2" << std::endl;
        fout << "\t.type " << global_val.val.name << ", @object" << std::endl;
        fout << "\t.size " << global_val.val.name << ", 4" << std::endl;
        fout << global_val.val.name << ":" << std::endl;
        fout << "\t.word "
             << "0" << std::endl
             << std::endl;
    }
    for (auto &func : program.functions)
    {
        gen_func(func);
    }
}

void backend::Generator::gen_func(const ir::Function &) {}
void backend::Generator::gen_instr(const ir::Instruction &) {}
