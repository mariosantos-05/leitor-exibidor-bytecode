#pragma once

#include "bytecode_viewer/ClassStructures.hpp"
#include "bytecode_viewer/ConstantPool.hpp"

#include <istream>
#include <string>
#include <vector>

/*
 * ClassReader.hpp
 * Parser principal do formato ClassFile da JVM.
 *
 * Este componente lê o arquivo .class em binário e monta a estrutura `ClassFile`
 * em memória. Ele percorre a ordem correta da especificação da JVM e resolve
 * os elementos relevantes como constant pool, campos, métodos e atributos.
 */

// Representação final de um arquivo .class já lido.
// Os campos correspondem aos blocos principais da estrutura oficial do ClassFile.
struct ClassFile {
    u4 magic;
    u2 minor_version;
    u2 major_version;
    ConstantPool constant_pool;
    u2 access_flags;
    u2 this_class;
    u2 super_class;
    std::vector<u2> interfaces;
    std::vector<FieldInfo> fields;
    std::vector<MethodInfo> methods;
    std::vector<AttributeInfo> attributes;

    ClassFile()
        : magic(0), minor_version(0), major_version(0), access_flags(0), this_class(0), super_class(0) {}

    // Funções auxiliares para obter nomes legíveis da classe atual e da superclasse.
    std::string thisClassName() const;
    std::string superClassName() const;
};

// Classe responsável por ler ClassFile a partir de um fluxo ou de um arquivo.
// Erros de leitura ou de parsing geram exceções std::runtime_error.
class ClassReader {
public:
    // Lê um arquivo .class a partir de um fluxo binário.
    ClassFile read(std::istream &input) const;

    // Abre um caminho em disco e faz a leitura do arquivo.
    ClassFile readFile(const std::string &path) const;

private:
    // Helpers internos usados para extrair atributos aninhados e Code.
    static AttributeInfo readAttribute(std::istream &input);
    static CodeAttribute readCodeAttribute(std::istream &input);
};