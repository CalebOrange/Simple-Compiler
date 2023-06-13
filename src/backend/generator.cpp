#include "backend/generator.h"

#include <assert.h>
#include <set>
#include <vector>
#include <iostream>

#define TODO assert(0 && "todo")

const std::vector<rv::rvREG> returnArgs{rv::rvREG::X10, rv::rvREG::X11, rv::rvREG::X12, rv::rvREG::X13, rv::rvREG::X14, rv::rvREG::X15, rv::rvREG::X16, rv::rvREG::X17};

int backend::stackVarMap::add_operand(ir::Operand operand, uint32_t size)
{
    _table[operand.name] = _table.size() * 4;
    return _table[operand.name];
}

int backend::stackVarMap::find_operand(ir::Operand operand)
{
    return _table[operand.name];
}

backend::Generator::Generator(ir::Program &p, std::ofstream &f) : program(p), fout(f) {}

rv::rvREG backend::Generator::getRd(ir::Operand operand)
{
    return rv::rvREG::X7;
}

rv::rvREG backend::Generator::getRs1(ir::Operand operand)
{
    load(operand, rv::rvREG::X5);
    return rv::rvREG::X5;
}

rv::rvREG backend::Generator::getRs2(ir::Operand operand)
{
    load(operand, rv::rvREG::X6);
    return rv::rvREG::X6;
}

void backend::Generator::load(ir::Operand operand, rv::rvREG reg, int offset)
{
    // if it is a global variable
    if (global_vals.count(operand.name))
    {
        fout << "\tlw " << toString(reg) << ", " << operand.name << std::endl;
        return;
    }
    fout << "\tlw " << toString(reg) << ", " << stackVar.find_operand(operand) << "(sp)" << std::endl;
}

void backend::Generator::store(ir::Operand operand, rv::rvREG reg, int offset)
{
    if (global_vals.count(operand.name))
    {
        fout << "\tsw " << toString(reg) << ", " << operand.name << ", t3" << std::endl;
        return;
    }
    fout << "\tsw " << toString(reg) << ", " << stackVar.find_operand(operand) << "(sp)" << std::endl;
}

void backend::Generator::gen()
{
    // generate global variables
    fout << "\t.data" << std::endl;
    for (auto &global_val : program.globalVal)
    {
        global_vals.insert(global_val.val.name);
        fout << "\t.global " << global_val.val.name << std::endl;
        fout << "\t.type " << global_val.val.name << ", @object" << std::endl;
        fout << "\t.size " << global_val.val.name << ", " << (global_val.maxlen ? global_val.maxlen * 4 : 4) << std::endl;
        fout << global_val.val.name << ":" << std::endl;
        fout << "\t.word "
             << "0" << std::endl
             << std::endl;
    }
    // generate functions
    for (auto &func : program.functions)
    {
        gen_func(func);
    }
    fout.close();
}

int get_frame_size(const ir::Function &func, backend::stackVarMap &stackVar)
{
    auto &params = func.ParameterList;
    auto &inst_vec = func.InstVec;
    int frame_size = 0;
    std::set<std::string> var_set;
    // parameters
    for (auto &param : params)
    {
        if (var_set.count(param.name))
            continue; // avoid duplicate (struct)
        var_set.insert(param.name);
        stackVar.add_operand(param, 4);
        std::cout << param.name << std::endl;
    }
    // local variable
    for (auto &ins : inst_vec)
    {
        if (var_set.count(ins->des.name))
            continue; // avoid duplicate (struct)
        var_set.insert(ins->des.name);
        stackVar.add_operand(ins->des, 4);
        std::cout << ins->des.name << std::endl;
    }
    frame_size += var_set.size() * 4;
    // special register
    frame_size += 4; // ra
    return frame_size;
}

void backend::Generator::gen_func(const ir::Function &function)
{
    // header
    fout << "\t.text" << std::endl;
    fout << "\t.global " << function.name << std::endl;
    fout << "\t.type " << function.name << ", @function" << std::endl;
    fout << function.name << ":" << std::endl;
    // body
    // entry
    stackVar._table.clear();
    int frame_size = get_frame_size(function, stackVar);
    fout << "\taddi sp, sp, -" << frame_size << std::endl;
    fout << "\tsw ra, " << frame_size - 4 << "(sp)" << std::endl;
    // generate
    for (auto &ins : function.InstVec)
    {
        gen_instr(*ins);
    }
    // exit
    fout << "\tlw ra, " << frame_size - 4 << "(sp)" << std::endl;
    fout << "\taddi sp, sp, " << frame_size << std::endl;
    fout << "\tjr ra" << std::endl;
    // footer
    fout << "\t.size " << function.name << ", .-" << function.name << std::endl;
}

void backend::Generator::gen_instr(const ir::Instruction &instruction)
{
    auto &op = instruction.op;
    auto &des = instruction.des;
    auto &src1 = instruction.op1;
    auto &src2 = instruction.op2;
    switch (op)
    {
    case ir::Operator::call:
    {
        auto call_inst = dynamic_cast<const ir::CallInst *>(&instruction);
        // arguments
        auto arguments_size = call_inst->argumentList.size();
        for (size_t i = 0; i < arguments_size; ++i)
        {
            auto &arg = call_inst->argumentList[i];
            auto &arg_reg = returnArgs[i];
            switch (arg.type)
            {
            case ir::Type::Int:
            case ir::Type::IntPtr:
            {
                auto reg = getRs1(arg);
                fout << "\tmv " << toString(arg_reg) << ", " << toString(reg) << std::endl;
                break;
            }
            case ir::Type::IntLiteral:
                fout << "\tli " << toString(arg_reg) << ", " << arg.name << std::endl;
                break;
            default:
                assert(0 && "wrong type");
                break;
            }
        }
        //   TODO: if parameter is too much, use stack to store
        // call
        fout << "\tcall " << call_inst->op1.name << std::endl;
        // save return value
        switch (des.type)
        {
        case ir::Type::Int:
        case ir::Type::IntPtr:
        {
            auto des_reg = getRd(des);
            fout << "\tmv " << toString(des_reg) << ", " << toString(rv::rvREG::X10) << std::endl;
            store(des, des_reg);
            break;
        }
        case ir::Type::null:
            break;
        default:
            assert(0 && "wrong type");
            break;
        }
        break;
    }
    case ir::Operator::_return:
    {
        switch (src1.type)
        {
        case ir::Type::Int:
        case ir::Type::IntPtr:
        {
            auto src1_reg = getRs1(src1);
            fout << "\tmv " << toString(rv::rvREG::X10) << ", " << toString(src1_reg) << std::endl;
            break;
        }
        case ir::Type::IntLiteral:
            fout << "\tli " << toString(rv::rvREG::X10) << ", " << src1.name << std::endl;
            break;
        case ir::Type::null:
            break;
        default:
            assert(0 && "wrong type");
            break;
        }
        break;
    }
    case ir::Operator::def:
    case ir::Operator::mov:
    {
        auto des_reg = getRd(des);
        switch (src1.type)
        {
        case ir::Type::Int:
        {
            auto src1_reg = getRs1(src1);
            fout << "\tmv " << toString(des_reg) << ", " << toString(src1_reg) << std::endl;
            store(des, des_reg);
            break;
        }
        case ir::Type::IntLiteral:
            fout << "\tli " << toString(des_reg) << ", " << src1.name << std::endl;
            store(des, des_reg);
            break;
        default:
            assert(0 && "wrong type");
            break;
        }
        break;
    }
    case ir::Operator::add:
    {
        auto des_reg = getRd(des);
        if (src1.type == ir ::Type::Int && src2.type == ir::Type::Int)
        {
            auto src1_reg = getRs1(src1);
            auto src2_reg = getRs2(src2);
            fout << "\tadd " << toString(des_reg) << ", " << toString(src1_reg) << ", " << toString(src2_reg) << std::endl;
            store(des, des_reg);
        }
        else if (src1.type == ir ::Type::IntLiteral && src2.type == ir::Type::Int)
        {
            auto src2_reg = getRs1(src2);
            fout << "\taddi " << toString(des_reg) << ", " << toString(src2_reg) << ", " << src1.name << std::endl;
            store(des, des_reg);
        }
        else
        {
            auto src1_reg = getRs1(src1);
            fout << "\taddi " << toString(des_reg) << ", " << toString(src1_reg) << ", " << src2.name << std::endl;
            store(des, des_reg);
        }

        break;
    }
    default:
        std::cout << "op: " << ir::toString(op) << std::endl;
        // assert(0 && "todo");
        break;
    }
}
