#include "bytecode_viewer/ClassReader.hpp"
#include "bytecode_viewer/BytecodeDecoder.hpp"

#include <cassert>
#include <iostream>
#include <vector>

// Verifica que o atributo Code foi corretamente lido para um método com bytecode.
void testCodeAttributeParsed() {
    ClassReader reader;
    ClassFile class_file = reader.readFile("../examples/soma_certo.class");

    bool has_method_with_code = false;
    for (const MethodInfo &method : class_file.methods) {
        if (method.has_code) {
            has_method_with_code = true;
            assert(method.code.max_stack > 0 && "max_stack deve ser maior que 0");
            assert(method.code.max_locals > 0 && "max_locals deve ser maior que 0");
            assert(!method.code.code.empty() && "bytecode do método não pode estar vazio");
        }
    }

    assert(has_method_with_code && "deve haver ao menos um método com atributo Code");
    std::cout << "[PASS] testCodeAttributeParsed\n";
}

// Verifica que vários métodos com código são parseados corretamente.
void testMultipleMethodsWithCode() {
    ClassReader reader;
    ClassFile class_file = reader.readFile("../examples/method_test.class");

    int methods_with_code = 0;
    for (const MethodInfo &method : class_file.methods) {
        if (method.has_code) {
            ++methods_with_code;
            assert(!method.code.code.empty() && "bytecode não pode ser vazio");
        }
    }

    assert(methods_with_code >= 1 && "deve haver ao menos um método com código");
    std::cout << "[PASS] testMultipleMethodsWithCode\n";
}

// Verifica que a tabela de exceções é parseada sem causar erro em métodos válidos.
void testExceptionTableParsed() {
    ClassReader reader;
    ClassFile class_file = reader.readFile("../examples/soma_certo.class");

    for (const MethodInfo &method : class_file.methods) {
        if (method.has_code) {
            for (const ExceptionTableEntry &entry : method.code.exception_table) {
                assert(entry.start_pc <= entry.end_pc && "start_pc deve ser <= end_pc");
            }
        }
    }

    std::cout << "[PASS] testExceptionTableParsed\n";
}

// Testes de sanidade e inicialização dos componentes básicos.
void testSanity() {
    ConstantPool pool;
    BytecodeDecoder decoder;
    ClassReader reader;

    assert(pool.empty());
    assert(decoder.decode(std::vector<u1>{}).empty());
    (void)reader;
    std::cout << "[PASS] testSanity\n";
}

// Testa a decodificação básica de instruções do BytecodeDecoder.
void testBytecodeDecoder() {
    BytecodeDecoder decoder;

    // Instruções simples sem operandos: aload_0 = 0x2A, return = 0xB1
    std::vector<u1> simple_code = {0x2A, 0xB1};
    auto instructions = decoder.decode(simple_code);
    assert(instructions.size() == 2);
    assert(instructions[0].mnemonic == "aload_0");
    assert(instructions[0].length == 1);
    assert(instructions[1].mnemonic == "return");

    // Instrução com operando: bipush = 0x10, argumento 0x05 -> valor 5
    std::vector<u1> operand_code = {0x10, 0x05};
    instructions = decoder.decode(operand_code);
    assert(instructions.size() == 1);
    assert(instructions[0].mnemonic == "bipush");
    assert(instructions[0].operand_text == "5");
    assert(instructions[0].length == 2);

    // Referência à constant pool: getstatic = 0xB2, args: 0x00, 0x02 -> #2
    std::vector<u1> constant_pool_code = {0xB2, 0x00, 0x02};
    instructions = decoder.decode(constant_pool_code);
    assert(instructions.size() == 1);
    assert(instructions[0].mnemonic == "getstatic");
    assert(instructions[0].operand_text == "#2");
    assert(instructions[0].length == 3);

    std::cout << "[PASS] testBytecodeDecoder\n";
}

int main() {
    testSanity();
    testCodeAttributeParsed();
    testMultipleMethodsWithCode();
    testExceptionTableParsed();
    testBytecodeDecoder();

    std::cout << "\nTodos os testes passaram.\n";
    return 0;
}
