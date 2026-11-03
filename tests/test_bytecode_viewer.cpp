#include "bytecode_viewer/ClassReader.hpp"
#include "bytecode_viewer/BytecodeDecoder.hpp"

#include <cassert>
#include <iostream>
#include <vector>

// Lê o arquivo soma_certo.class (exemplo simples com um método com código)
// e verifica que o CodeAttribute foi parseado corretamente.
void test_code_attribute_parsed() {
    ClassReader reader;
    ClassFile cf = reader.readFile("../exemplos/soma_certo.class");

    // O arquivo deve ter ao menos um método com código
    bool found_code = false;
    for (const MethodInfo& method : cf.methods) {
        if (method.has_code) {
            found_code = true;
            // max_stack e max_locals devem ser valores razoáveis (> 0)
            assert(method.code.max_stack > 0 && "max_stack deve ser maior que 0");
            assert(method.code.max_locals > 0 && "max_locals deve ser maior que 0");
            // O bytecode não pode estar vazio
            assert(!method.code.code.empty() && "código do método não pode ser vazio");
        }
    }
    assert(found_code && "deve haver ao menos um método com atributo Code");
    std::cout << "[PASS] test_code_attribute_parsed\n";
}

// Verifica que múltiplos métodos com código são parseados (arquivo com vários métodos)
void test_multiple_methods_with_code() {
    ClassReader reader;
    ClassFile cf = reader.readFile("../exemplos/method_test.class");

    int methods_with_code = 0;
    for (const MethodInfo& method : cf.methods) {
        if (method.has_code) {
            methods_with_code++;
            assert(!method.code.code.empty() && "bytecode não pode ser vazio");
        }
    }
    assert(methods_with_code >= 1 && "deve haver ao menos um método com código");
    std::cout << "[PASS] test_multiple_methods_with_code\n";
}

// Verifica que a exception_table é parseada (pode estar vazia para métodos sem try/catch)
void test_exception_table_parsed() {
    ClassReader reader;
    ClassFile cf = reader.readFile("../exemplos/soma_certo.class");

    for (const MethodInfo& method : cf.methods) {
        if (method.has_code) {
            // exception_table pode ser vazia, mas não deve causar erro
            for (const ExceptionTableEntry& e : method.code.exception_table) {
                assert(e.start_pc <= e.end_pc && "start_pc deve ser <= end_pc");
            }
        }
    }
    std::cout << "[PASS] test_exception_table_parsed\n";
}

// Sanity checks originais mantidos
void test_sanity() {
    ConstantPool pool;
    BytecodeDecoder decoder;
    ClassReader reader;

    assert(pool.empty());
    assert(decoder.decode(std::vector<u1>{}).empty());
    (void)reader;
    std::cout << "[PASS] test_sanity\n";
}

// Testa a decodificação básica de instruções pelo BytecodeDecoder
void test_bytecode_decoder() {
    BytecodeDecoder decoder;
    
    // Teste 1: Instruções simples sem operandos (aload_0 = 0x2A, return = 0xB1)
    std::vector<u1> simple_code = {0x2A, 0xB1};
    auto instrs = decoder.decode(simple_code);
    assert(instrs.size() == 2);
    assert(instrs[0].mnemonic == "aload_0");
    assert(instrs[0].length == 1);
    assert(instrs[1].mnemonic == "return");
    
    // Teste 2: Instrução com operando (bipush = 0x10, args: 0x05)
    std::vector<u1> operand_code = {0x10, 0x05};
    instrs = decoder.decode(operand_code);
    assert(instrs.size() == 1);
    assert(instrs[0].mnemonic == "bipush");
    assert(instrs[0].operand_text == "5");
    assert(instrs[0].length == 2);
    
    // Teste 3: Referência ao Constant Pool (getstatic = 0xB2, args: 0x00, 0x02 -> #2)
    std::vector<u1> cp_code = {0xB2, 0x00, 0x02};
    instrs = decoder.decode(cp_code);
    assert(instrs.size() == 1);
    assert(instrs[0].mnemonic == "getstatic");
    assert(instrs[0].operand_text == "#2");
    assert(instrs[0].length == 3);

    std::cout << "[PASS] test_bytecode_decoder\n";
}

int main() {
    test_sanity();
    test_code_attribute_parsed();
    test_multiple_methods_with_code();
    test_exception_table_parsed();
    test_bytecode_decoder();

    std::cout << "\nTodos os testes passaram.\n";
    return 0;
}
