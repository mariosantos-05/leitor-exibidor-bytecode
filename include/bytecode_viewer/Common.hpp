#pragma once

#include <cstddef>
#include <cstdint>
#include <istream>
#include <stdexcept>
#include <string>
#include <vector>

/*
 * Common.hpp
 * Utilitários básicos para leitura de arquivos .class em formato binário.
 *
 * O formato JVM usa encoding big-endian e tipos fixos de tamanho. Este arquivo
 * centraliza aliases e funções de leitura para que o restante do projeto possa
 * operar sobre o bytecode com uma API simples e legível.
 *
 * Tipos principais:
 *  - u1: 8 bits sem sinal (equivalente a byte em Java)
 *  - u2: 16 bits sem sinal
 *  - u4: 32 bits sem sinal
 */

using u1 = std::uint8_t;
using u2 = std::uint16_t;
using u4 = std::uint32_t;

// Lê 1 byte sem sinal do fluxo de entrada.
// Lança std::runtime_error em caso de fim inesperado do arquivo.
inline u1 readU1(std::istream &input) {
    char value = 0;
    if (!input.get(value)) {
        throw std::runtime_error("unexpected end of stream while reading u1");
    }
    return static_cast<u1>(static_cast<unsigned char>(value));
}

// Lê 2 bytes em ordem big-endian e devolve como u2.
inline u2 readU2(std::istream &input) {
    return static_cast<u2>((static_cast<u2>(readU1(input)) << 8) |
                           static_cast<u2>(readU1(input)));
}

// Lê 4 bytes em ordem big-endian e devolve como u4.
inline u4 readU4(std::istream &input) {
    return static_cast<u4>((static_cast<u4>(readU2(input)) << 16) |
                           static_cast<u4>(readU2(input)));
}

// Lê `count` bytes brutos do fluxo e devolve um vetor de u1.
inline std::vector<u1> readBytes(std::istream &input, std::size_t count) {
    std::vector<u1> bytes(count);
    if (count == 0) {
        return bytes;
    }

    input.read(reinterpret_cast<char *>(bytes.data()),
               static_cast<std::streamsize>(count));
    if (input.gcount() != static_cast<std::streamsize>(count)) {
        throw std::runtime_error("unexpected end of stream while reading bytes");
    }

    return bytes;
}

// Converte bytes crus em std::string para representar textos UTF-8 presentes
// na constant pool e em outros trechos do arquivo de classe.
inline std::string bytesToString(const std::vector<u1> &bytes) {
    return std::string(bytes.begin(), bytes.end());
}