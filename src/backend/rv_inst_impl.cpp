#include "backend/rv_inst_impl.h"
#include <cassert>

std::string rv::rv_inst::draw() const
{
    switch (op)
    {
    case rvOPCODE::ADD:
    case rvOPCODE::SUB:
    case rvOPCODE::MUL:
    case rvOPCODE::DIV:
        return toString(op) + " " + toString(rd) + ", " + toString(rs1) + ", " + toString(rs2);
        break;
    case rvOPCODE::ADDI:
        return toString(op) + " " + toString(rd) + ", " + toString(rs1) + ", " + std::to_string(imm);
        break;
    case rvOPCODE::SW:
        return toString(op) + " " + toString(rs2) + ", " + std::to_string(imm) + "(" + toString(rs1) + ")";
        break;
    default:
        break;
    }
}