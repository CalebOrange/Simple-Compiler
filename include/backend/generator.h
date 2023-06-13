#ifndef GENERARATOR_H
#define GENERARATOR_H

#include "ir/ir.h"
#include "backend/rv_def.h"
#include "backend/rv_inst_impl.h"

#include<map>
#include<string>
#include<vector>
#include<fstream>
#include<set>

namespace backend {

// it is a map bewteen variable and its mem addr, the mem addr of a local variable can be identified by ($sp + off)
struct stackVarMap {
    std::map<std::string, int> _table;

    /**
     * @brief find the addr of a ir::Operand
     * @return the offset
    */
    int find_operand(ir::Operand);

    /**
     * @brief add a ir::Operand into current map, alloc space for this variable in memory 
     * @param[in] size: the space needed(in byte)
     * @return the offset
    */
    int add_operand(ir::Operand, uint32_t size = 4);
};


struct Generator {
    const ir::Program& program;         // the program to gen
    std::ofstream& fout;                // output file
    stackVarMap stackVar;               // the stackVarMap of current function
    std::set<std::string> global_vals; // the global variables

    Generator(ir::Program&, std::ofstream&);

    // reg allocate api
    rv::rvREG getRd(ir::Operand);   // get a reg for a ir::Operand
    rv::rvFREG fgetRd(ir::Operand);
    rv::rvREG getRs1(ir::Operand);  // get a reg for a ir::Operand
    rv::rvREG getRs2(ir::Operand);  // get a reg for a ir::Operand
    rv::rvFREG fgetRs1(ir::Operand);
    rv::rvFREG fgetRs2(ir::Operand);

    // load/store api
    void load(ir::Operand, rv::rvREG, int offset=0); // load a word from mem
    void store(ir::Operand, rv::rvREG, int offset=0); // store a word to mem

    // generate wrapper function
    void gen(); // generate the whole program
    void gen_func(const ir::Function&); // generate a function
    void gen_instr(const ir::Instruction&); // generate a instruction
};



} // namespace backend


#endif