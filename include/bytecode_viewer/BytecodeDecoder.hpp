#pragma once

#include "bytecode_viewer/Common.hpp"

#include <string>
#include <vector>

/*
 * BytecodeDecoder.hpp
 * Small utility to convert raw JVM bytecode (a sequence of opcodes) into
 * a human-readable list of instructions. This is intentionally simple: it
 * maps opcodes to mnemonics and formats operands as text for display.
 */

// A single decoded instruction with program-counter, opcode, mnemonic and
// textual operand representation. Useful for printing or simple analysis.
struct Instruction {
    std::size_t pc;         // byte offset inside the code array
    u1 opcode;              // raw opcode value
    std::string mnemonic;   // human-readable opcode name (e.g., "aload_0")
    std::string operand_text;// textual representation of operands (if any)
    std::size_t length;     // length in bytes of this instruction

    Instruction()
        : pc(0), opcode(0), length(0) {}
};

// BytecodeDecoder: high-level decoder that turns a byte vector into
// a list of `Instruction`. The implementation uses simple tables to
// resolve mnemonics and operand sizes.
class BytecodeDecoder {
public:
    std::string mnemonic(u1 opcode) const;
    std::vector<Instruction> decode(const std::vector<u1> &code) const;
};