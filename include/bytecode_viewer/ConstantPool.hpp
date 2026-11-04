#pragma once

#include "bytecode_viewer/Common.hpp"

#include <string>
#include <vector>

/*
 * ConstantPool.hpp
 * Define a tabela de constantes do arquivo .class.
 *
 * A JVM armazena nomes, tipos, referências de campos e métodos e outros dados
 * em uma estrutura chamada constant pool. Cada entrada possui uma tag que indica
 * seu tipo, e esta classe facilita a leitura e a conversão dessas entradas para
 * texto legível.
 */

// Uma entrada individual da constant pool.
// Dependendo da tag, alguns campos são usados e outros permanecem vazios.
struct CpInfo {
    u1 tag;
    std::string utf8_value;
    u2 index1;
    u2 index2;
    std::vector<u1> bytes;

    CpInfo()
        : tag(0), index1(0), index2(0) {}
};

// Facilita a consulta e a resolução de entradas da constant pool.
// A implementação fornece acesso direto, conversões úteis e formatação textual
// para depuração e apresentação do conteúdo do .class.
class ConstantPool {
public:
    ConstantPool();
    explicit ConstantPool(std::vector<CpInfo> entries);

    // Quantidade de entradas válidas da constant pool (sem contar o índice 0).
    std::size_t size() const;
    bool empty() const;

    // Acesso direto a uma entrada usando índice 1-based, conforme a JVM.
    const CpInfo &at(u2 index) const;

    // Helpers para interpretar entradas comuns da constant pool.
    std::string utf8(u2 index) const;
    std::string className(u2 index) const;
    std::string nameAndType(u2 index) const;
    std::string toString(u2 index) const;

private:
    std::vector<CpInfo> entries_;
};