#pragma once

#include "bytecode_viewer/Common.hpp"

#include <string>
#include <vector>

/*
 * BytecodeDecoder.hpp
 * Decodificação de instruções JVM para leitura humana do bytecode.
 *
 * Este componente recebe um vetor de bytes correspondente ao atributo Code de
 * um método e transforma cada opcode em uma instrução legível, com mnemonic,
 * operandos e posição do programa (pc).
 */

// Representa uma instrução já decodificada.
// O `pc` indica a posição inicial no array de instruções e `length` indica
// quantos bytes a instrução ocupa na sequência do bytecode.
struct Instruction {
    std::size_t pc;
    u1 opcode;
    std::string mnemonic;
    std::string operand_text;
    std::size_t length;

    Instruction()
        : pc(0), opcode(0), length(0) {}
};

// Decodifica bytecode JVM em uma lista de instruções facilmente exibíveis.
class BytecodeDecoder {
public:
    std::string mnemonic(u1 opcode) const;
    std::vector<Instruction> decode(const std::vector<u1> &code) const;
};