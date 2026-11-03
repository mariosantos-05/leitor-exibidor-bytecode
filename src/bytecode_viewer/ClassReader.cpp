#include "bytecode_viewer/ClassReader.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

CpInfo readNextConstantPoolEntry(std::istream &input) {
    CpInfo entry;
    entry.tag = readU1(input);
    
    switch (entry.tag) {
        case 1: { // CONSTANT_Utf8
            u2 length = readU2(input);
            std::vector<u1> bytes = readBytes(input, length);
            // Converte o vetor de bytes diretamente para uma string C++
            entry.utf8_value = std::string(bytes.begin(), bytes.end());
            break;
        }
        case 7:   // CONSTANT_Class
        case 8:   // CONSTANT_String
        case 16:  // CONSTANT_MethodType
            // Armazena o índice u2 que aponta para um CONSTANT_Utf8
            entry.index1 = readU2(input);
            break;
        case 3:   // CONSTANT_Integer
        case 4:   // CONSTANT_Float
            // Armazena os 4 bytes brutos do valor numérico
            entry.bytes = readBytes(input, 4);
            break;
        case 5:   // CONSTANT_Long
        case 6:   // CONSTANT_Double
            // Armazena os 8 bytes brutos do valor numérico de precisão dupla
            entry.bytes = readBytes(input, 8);
            break;
        case 9:   // CONSTANT_Fieldref
        case 10:  // CONSTANT_Methodref
        case 11:  // CONSTANT_InterfaceMethodref
        case 12:  // CONSTANT_NameAndType
        case 18:  // CONSTANT_InvokeDynamic
            // Estruturas de associação: guardam dois índices apontando para outras tags
            entry.index1 = readU2(input);
            entry.index2 = readU2(input);
            break;
        case 15:  // CONSTANT_MethodHandle
            entry.index1 = readU1(input); // reference_kind (1 byte)
            entry.index2 = readU2(input); // reference_index (2 bytes)
            break;
        default:
            throw std::runtime_error("Unknown Constant Pool tag: " + std::to_string(static_cast<int>(entry.tag)));
    }
    return entry;
}

AttributeInfo ClassReader::readAttribute(std::istream &input) {
    AttributeInfo attr;
    attr.name_index = readU2(input);
    u4 length = readU4(input);
    attr.info = readBytes(input, length);
    return attr;
}

// Lê o conteúdo do atributo Code
// O stream deve estar posicionado logo após o campo attribute_length,
// apontando para o início do payload (max_stack).
CodeAttribute ClassReader::readCodeAttribute(std::istream &input) {
    CodeAttribute code;

    // Lê os limites de pilha e variáveis locais usados pelo método
    code.max_stack  = readU2(input);
    code.max_locals = readU2(input);

    // Lê os bytes do bytecode
    u4 code_length = readU4(input);
    code.code = readBytes(input, code_length);

    // Lê a tabela de exceções (handlers de try/catch/finally)
    u2 exception_table_length = readU2(input);
    code.exception_table.reserve(exception_table_length);
    for (u2 i = 0; i < exception_table_length; ++i) {
        ExceptionTableEntry entry;
        entry.start_pc   = readU2(input); // início do bloco protegido (inclusive)
        entry.end_pc     = readU2(input); // fim do bloco protegido (exclusive)
        entry.handler_pc = readU2(input); // instrução inicial do handler
        entry.catch_type = readU2(input); // índice na CP (0 = finally)
        code.exception_table.push_back(entry);
    }

    // Lê atributos aninhados (ex: LineNumberTable, LocalVariableTable)
    u2 attributes_count = readU2(input);
    code.attributes.reserve(attributes_count);
    for (u2 i = 0; i < attributes_count; ++i) {
        code.attributes.push_back(readAttribute(input));
    }

    return code;
}

ClassFile ClassReader::read(std::istream &input) const {
    ClassFile cf;

    // 1. Número Mágico (Magic Number)
    cf.magic = readU4(input);
    if (cf.magic != 0xCAFEBABE) {
        throw std::runtime_error("Invalid class file: magic number 0xCAFEBABE not found.");
    }

    // 2. Versões (Minor e Major)
    cf.minor_version = readU2(input);
    cf.major_version = readU2(input);

    // 3. Pool de Constantes (Constant Pool)
    u2 cp_count = readU2(input);
    std::vector<CpInfo> cp_entries;
    // O índice da CP começa em 1.
    cp_entries.push_back(CpInfo()); 

    for (u2 i = 1; i < cp_count; ++i) {
        CpInfo entry = readNextConstantPoolEntry(input);
        cp_entries.push_back(entry);

        // Especificação técnica da JVM: CONSTANT_Long (5) e CONSTANT_Double (6)
        // ocupam dois espaços consecutivos na tabela de constantes.
        if (entry.tag == 5 || entry.tag == 6) {
            cp_entries.push_back(CpInfo());
            i++; 
        }
    }
    cf.constant_pool = ConstantPool(cp_entries);

    // 4. Flags de Acesso e Informações da Classe
    cf.access_flags = readU2(input);
    cf.this_class = readU2(input);
    cf.super_class = readU2(input);

    // 5. Interfaces
    u2 interfaces_count = readU2(input);
    cf.interfaces.reserve(interfaces_count);
    for (u2 i = 0; i < interfaces_count; ++i) {
        cf.interfaces.push_back(readU2(input));
    }

    // 6. Campos (Fields)
    u2 fields_count = readU2(input);
    cf.fields.reserve(fields_count);
    for (u2 i = 0; i < fields_count; ++i) {
        FieldInfo field;
        field.access_flags = readU2(input);
        field.name_index = readU2(input);
        field.descriptor_index = readU2(input);
        
        u2 attr_count = readU2(input);
        field.attributes.reserve(attr_count);
        for (u2 j = 0; j < attr_count; ++j) {
            field.attributes.push_back(readAttribute(input));
        }
        cf.fields.push_back(field);
    }

    // 7. Métodos (Methods)
    u2 methods_count = readU2(input);
    cf.methods.reserve(methods_count);
    for (u2 i = 0; i < methods_count; ++i) {
        MethodInfo method;
        method.access_flags     = readU2(input);
        method.name_index       = readU2(input);
        method.descriptor_index = readU2(input);
        method.has_code         = false;

        u2 attr_count = readU2(input);
        method.attributes.reserve(attr_count);
        for (u2 j = 0; j < attr_count; ++j) {
            // Lemos o atributo genérico primeiro para obter nome e bytes raw
            AttributeInfo attr = readAttribute(input);

            // Verifica se este atributo é o "Code" pelo nome na constant pool
            std::string attr_name = cf.constant_pool.utf8(attr.name_index);
            if (attr_name == "Code") {
                // Reinterpreta os bytes raw como um CodeAttribute usando um
                // stringstream para reutilizar as funções de leitura existentes
                std::string raw(attr.info.begin(), attr.info.end());
                std::istringstream code_stream(raw);
                method.code     = readCodeAttribute(code_stream);
                method.has_code = true;
            } else {
                // Atributos não-Code ficam como bytes raw em method.attributes
                method.attributes.push_back(attr);
            }
        }
        cf.methods.push_back(method);
    }

    // 8. Atributos Globais da Classe
    u2 attributes_count = readU2(input);
    cf.attributes.reserve(attributes_count);
    for (u2 i = 0; i < attributes_count; ++i) {
        cf.attributes.push_back(readAttribute(input));
    }

    return cf;
}

ClassFile ClassReader::readFile(const std::string &path) const {
    std::ifstream input(path.c_str(), std::ios::binary);
    if (!input) {
        throw std::runtime_error("unable to open class file: " + path);
    }
    return read(input);
}

std::string ClassFile::thisClassName() const {
    if (this_class == 0) return std::string();
    return constant_pool.className(this_class);
}

std::string ClassFile::superClassName() const {
    if (super_class == 0) return std::string();
    return constant_pool.className(super_class);
}