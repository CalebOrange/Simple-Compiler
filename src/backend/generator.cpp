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
    return rv::rvREG::X5;
}

rv::rvREG backend::Generator::getRs1(ir::Operand operand)
{
    return rv::rvREG::X6;
}

rv::rvREG backend::Generator::getRs2(ir::Operand operand)
{
    return rv::rvREG::X7;
}

rv::rvREG backend::Generator::getAdrress(ir::Operand operand)
{

    return rv::rvREG::X28;
}

void backend::Generator::load(ir::Operand operand, rv::rvREG reg, int offset)
{
    // if it is a global variable
    if (global_vals.count(operand.name))
    {
        fout << "\tla " << toString(reg) << ", " << operand.name << std::endl;
        fout << "\tlw " << toString(reg) << ", " << offset << "(" << toString(reg) << ")" << std::endl;
        return;
    }
    fout << "\tlw " << toString(reg) << ", " << stackVar.find_operand(operand) + offset << "(sp)" << std::endl;
}

void backend::Generator::store(ir::Operand operand, rv::rvREG reg, int offset)
{
    if (global_vals.count(operand.name))
    {
        fout << "\tla " << toString(rv::rvREG::X28) << ", " << operand.name << std::endl; // t3 is a temp register
        fout << "\tsw " << toString(reg) << ", " << offset << "(" + toString(rv::rvREG::X28) + ")" << std::endl;
        return;
    }
    fout << "\tsw " << toString(reg) << ", " << stackVar.find_operand(operand) + offset << "(sp)" << std::endl;
}

void backend::Generator::fout_instr(rv::rvOPCODE op, rv::rvREG rd, rv::rvREG rs1, rv::rvREG rs2)
{
    auto inst = rv::rv_inst(op, rd, rs1, rs2);
    fout << inst.draw() << std::endl;
}

void backend::Generator::fout_instr(rv::rvOPCODE op, rv::rvREG rd, rv::rvREG rs1, uint32_t imm)
{
    auto inst = rv::rv_inst(op, rd, rs1, imm);
    fout << inst.draw() << std::endl;
}

void backend::Generator::fout_instr(rv::rvOPCODE op, rv::rvREG reg1, rv::rvREG reg2, uint32_t imm)
{
    auto inst = rv::rv_inst(op, reg1, reg2, imm);
    fout << inst.draw() << std::endl;
}

void backend::Generator::fout_instr(rv::rvOPCODE op, rv::rvREG rd, uint32_t imm)
{
    auto inst = rv::rv_inst(op, rd, imm);
    fout << inst.draw() << std::endl;
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
    std::cout << std::endl;
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
    }
    // local variable
    for (auto &ins : inst_vec)
    {
        if (var_set.count(ins->des.name))
            continue; // avoid duplicate (struct)
        if (ins->op == ir::Operator::alloc)
        {
            var_set.insert(ins->des.name);
            stackVar.add_operand(ins->des, std::stoi(ins->op1.name) * 4);
            frame_size += std::stoi(ins->op1.name) * 4;
            std::cout << ins->des.name << " : " << stackVar._table[ins->des.name] << std::endl;
            continue;
        }
        var_set.insert(ins->des.name);
        stackVar.add_operand(ins->des, 4);
        frame_size += 4;
        std::cout << ins->des.name << " : " << stackVar._table[ins->des.name] << std::endl;
    }
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
    uint32_t frame_size = get_frame_size(function, stackVar);
    fout << "\taddi sp, sp, -" << frame_size << std::endl;
    // fout_instr(rv::rvOPCODE::ADDI, rv::rvREG::X2, rv::rvREG::X2, rv::rvREG::X0, -frame_size, ""); // fp = sp
    fout << "\tsw ra, " << frame_size - 4 << "(sp)" << std::endl;
    // fout_instr(rv::rvOPCODE::SW, rv::rvREG::X1, rv::rvREG::X2, rv::rvREG::X1, frame_size - 4, ""); // ra = sp - 4
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
        fout << "# call" << std::endl;
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
                load(arg, reg);
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
        fout << "# return" << std::endl;
        switch (src1.type)
        {
        case ir::Type::Int:
        case ir::Type::IntPtr:
        {
            auto src1_reg = getRs1(src1);
            load(src1, src1_reg);
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
        fout << "# def/mov" << std::endl;
        auto des_reg = getRd(des);
        switch (src1.type)
        {
        case ir::Type::Int:
        {
            auto src1_reg = getRs1(src1);
            load(src1, src1_reg);
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
        fout << "# add" << std::endl;
        auto des_reg = getRd(des);
        if (src1.type == ir ::Type::Int && src2.type == ir::Type::Int)
        {
            auto src1_reg = getRs1(src1);
            load(src1, src1_reg);
            auto src2_reg = getRs2(src2);
            load(src2, src2_reg);
            fout << "\tadd " << toString(des_reg) << ", " << toString(src1_reg) << ", " << toString(src2_reg) << std::endl;
            store(des, des_reg);
        }
        else if (src1.type == ir ::Type::IntLiteral && src2.type == ir::Type::Int)
        {
            auto src2_reg = getRs1(src2);
            load(src2, src2_reg);
            fout << "\taddi " << toString(des_reg) << ", " << toString(src2_reg) << ", " << src1.name << std::endl;
            store(des, des_reg);
        }
        else
        {
            auto src1_reg = getRs1(src1);
            load(src1, src1_reg);
            fout << "\taddi " << toString(des_reg) << ", " << toString(src1_reg) << ", " << src2.name << std::endl;
            store(des, des_reg);
        }

        break;
    }
    case ir::Operator::mul:
    {
        fout << "# mul" << std::endl;
        auto des_reg = getRd(des);
        if (src1.type == ir ::Type::Int && src2.type == ir::Type::Int)
        {
            auto src1_reg = getRs1(src1);
            load(src1, src1_reg);
            auto src2_reg = getRs2(src2);
            load(src2, src2_reg);
            fout << "\tmul " << toString(des_reg) << ", " << toString(src1_reg) << ", " << toString(src2_reg) << std::endl;
        }
        else if (src1.type == ir ::Type::IntLiteral && src2.type == ir::Type::Int)
        {
            auto src2_reg = getRs1(src2);
            load(src2, src2_reg);
            fout << "\tli " << toString(rv::rvREG::X28) << ", " << src1.name << std::endl;
            fout << "\tmul " << toString(des_reg) << ", " << toString(src2_reg) << ", " << toString(rv::rvREG::X11) << std::endl;
        }
        else if (src1.type == ir ::Type::Int && src2.type == ir::Type::IntLiteral)
        {
            auto src1_reg = getRs1(src1);
            load(src1, src1_reg);
            fout << "\tli " << toString(rv::rvREG::X28) << ", " << src2.name << std::endl;
            fout << "\tmul " << toString(des_reg) << ", " << toString(src1_reg) << ", " << toString(rv::rvREG::X11) << std::endl;
        }
        else
        {
            fout << "\tli " << toString(rv::rvREG::X28) << ", " << src1.name << std::endl;
            fout << "\tli " << toString(rv::rvREG::X29) << ", " << src2.name << std::endl;
            fout << "\tmul " << toString(des_reg) << ", " << toString(rv::rvREG::X28) << ", " << toString(rv::rvREG::X29) << std::endl;
        }
        store(des, des_reg);
        break;
    }
    case ir::Operator::store:
    {
        fout << "# store" << std::endl;
        auto src1_reg = getRs1(src1); // address
        if (src2.type == ir::Type::IntLiteral)
        {
            if (des.type == ir::Type::IntLiteral)
            {
                fout << "\tli " << toString(rv::rvREG::X28) << ", " << des.name << std::endl;
                store(src1, rv::rvREG::X28, std::stoi(src2.name) * 4);
            }
            else if (des.type == ir::Type::Int)
            {
                auto des_reg = getRs2(des);
                load(des, des_reg);
                store(src1, des_reg, std::stoi(src2.name) * 4);
            }
            else
            {
                assert(0 && "wrong type");
            }
        }
        else if (src2.type == ir::Type::Int)
        {
            auto src2_reg = getRs2(src2);
            load(src2, src2_reg);
            fout << "\tslli " << toString(src2_reg) << ", " << toString(src2_reg) << ", 2" << std::endl;
            if (des.type == ir::Type::IntLiteral)
            {
                fout << "\tli " << toString(rv::rvREG::X28) << ", " << des.name << std::endl;
                fout << "\tadd " << toString(src1_reg) << ", " << toString(src1_reg) << ", " << toString(src2_reg) << std::endl;
                fout << "\tsw " << toString(rv::rvREG::X28) << ", 0(" << toString(src1_reg) << ")" << std::endl;
            }
            else if (des.type == ir::Type::Int)
            {
                auto des_reg = getRs2(des);
                load(des, des_reg);
                fout << "\tli " << toString(rv::rvREG::X28) << ", " << des.name << std::endl;
                fout << "\tadd " << toString(src1_reg) << ", " << toString(src1_reg) << ", " << toString(src2_reg) << std::endl;
                fout << "\tsw " << toString(des_reg) << ", 0(" << toString(src1_reg) << ")" << std::endl;
            }
            else
            {
                assert(0 && "wrong type");
            }
        }
        else
        {
            assert(0 && "wrong type");
        }
        break;
    }
    case ir::Operator::load:
    {
        fout << "# load" << std::endl;
        auto des_reg = getRd(des);
        // fout << "load" << std::endl;
        if (src2.type == ir::Type::IntLiteral)
        {
            auto src1_reg = getRs1(src1);
            fout << "\taddi " << toString(src1_reg) << ", " << toString(rv::rvREG::X2) << ", " << src2.name << std::endl;
            fout << "\tmv " << toString(des_reg) << ", " << toString(src1_reg) << std::endl;
            store(des, des_reg);
        }
        else
        {
            auto src1_reg = getRs1(src1); // address
            fout << "\taddi " << toString(src1_reg) << ", " << toString(rv::rvREG::X2) << ", " << stackVar.find_operand(src1) << std::endl;
            auto src2_reg = getRs2(src2); // offset
            load(src2, src2_reg);
            fout << "\tslli " << toString(src2_reg) << ", " << toString(src2_reg) << ", 2" << std::endl;
            fout << "\tadd " << toString(des_reg) << ", " << toString(src1_reg) << ", " << toString(src2_reg) << std::endl;
            fout << "\tlw " << toString(des_reg) << ", 0(" << toString(des_reg) << ")" << std::endl;
            store(des, des_reg);
        }
        break;
    }
    case ir::Operator::alloc:
    {
        break;
    }
    default:
        std::cout << "op: " << ir::toString(op) << std::endl;
        // assert(0 && "todo");
        break;
    }
}
