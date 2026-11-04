#pragma once

#include "bytecode_viewer/Common.hpp"

#include <vector>

/*
 * ClassStructures.hpp
 * Estruturas em memória que espelham partes do formato ClassFile da JVM.
 *
 * Estas structs representam os elementos fundamentais da especificação do
 * arquivo .class, mantidos em forma simples e legível para estudo e análise.
 * Elas são usadas como modelo de dados para o parser e para a exibição do
 * bytecode.
 */

// Representa um atributo genérico presente em um ClassFile, campo ou método.
// O campo `name_index` aponta para uma entrada UTF-8 na constant pool, e
// `info` armazena o conteúdo bruto do atributo em bytes.
struct AttributeInfo {
    u2 name_index;
    std::vector<u1> info;

    AttributeInfo()
        : name_index(0) {}
};

// Entrada da tabela de exceções usada no atributo Code.
// Cada item descreve um intervalo protegido e o ponto de destino do handler.
struct ExceptionTableEntry {
    u2 start_pc;
    u2 end_pc;
    u2 handler_pc;
    u2 catch_type; // índice da constant pool para a classe de exceção; 0 = finally

    ExceptionTableEntry()
        : start_pc(0), end_pc(0), handler_pc(0), catch_type(0) {}
};

// Atributo Code: guarda o bytecode do método, sua pilha, variáveis locais e
// handlers de exceção. Este é o trecho mais importante para a decodificação.
struct CodeAttribute {
    u2 max_stack;
    u2 max_locals;
    std::vector<u1> code;
    std::vector<ExceptionTableEntry> exception_table;
    std::vector<AttributeInfo> attributes;

    CodeAttribute()
        : max_stack(0), max_locals(0) {}
};

// Metadados de um campo da classe, incluindo nome, descritor e atributos.
struct FieldInfo {
    u2 access_flags;
    u2 name_index;
    u2 descriptor_index;
    std::vector<AttributeInfo> attributes;

    FieldInfo()
        : access_flags(0), name_index(0), descriptor_index(0) {}
};

// Metadados de um método. Quando o método possui código, a estrutura `code`
// armazena o atributo `Code` já decodificado.
struct MethodInfo {
    u2 access_flags;
    u2 name_index;
    u2 descriptor_index;
    bool has_code;
    CodeAttribute code;
    std::vector<AttributeInfo> attributes;

    MethodInfo()
        : access_flags(0), name_index(0), descriptor_index(0), has_code(false) {}
};