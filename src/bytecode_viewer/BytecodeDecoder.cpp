#include "bytecode_viewer/BytecodeDecoder.hpp"
#include <iomanip>
#include <sstream>

namespace {
    // Array com todos os nomes dos mnemônicos baseados na especificação da JVM.
    // O índice do array corresponde ao valor hexadecimal do opcode.
    const char* kMnemonics[256] = {
        "nop", "aconst_null", "iconst_m1", "iconst_0", "iconst_1", "iconst_2", "iconst_3", "iconst_4",
        "iconst_5", "lconst_0", "lconst_1", "fconst_0", "fconst_1", "fconst_2", "dconst_0", "dconst_1",
        "bipush", "sipush", "ldc", "ldc_w", "ldc2_w", "iload", "lload", "fload",
        "dload", "aload", "iload_0", "iload_1", "iload_2", "iload_3", "lload_0", "lload_1",
        "lload_2", "lload_3", "fload_0", "fload_1", "fload_2", "fload_3", "dload_0", "dload_1",
        "dload_2", "dload_3", "aload_0", "aload_1", "aload_2", "aload_3", "iaload", "laload",
        "faload", "daload", "aaload", "baload", "caload", "saload", "istore", "lstore",
        "fstore", "dstore", "astore", "istore_0", "istore_1", "istore_2", "istore_3", "lstore_0",
        "lstore_1", "lstore_2", "lstore_3", "fstore_0", "fstore_1", "fstore_2", "fstore_3", "dstore_0",
        "dstore_1", "dstore_2", "dstore_3", "astore_0", "astore_1", "astore_2", "astore_3", "iastore",
        "lastore", "fastore", "dastore", "aastore", "bastore", "castore", "sastore", "pop",
        "pop2", "dup", "dup_x1", "dup_x2", "dup2", "dup2_x1", "dup2_x2", "swap",
        "iadd", "ladd", "fadd", "dadd", "isub", "lsub", "fsub", "dsub",
        "imul", "lmul", "fmul", "dmul", "idiv", "ldiv", "fdiv", "ddiv",
        "irem", "lrem", "frem", "drem", "ineg", "lneg", "fneg", "dneg",
        "ishl", "lshl", "ishr", "lshr", "iushr", "lushr", "iand", "land",
        "ior", "lor", "ixor", "lxor", "iinc", "i2l", "i2f", "i2d",
        "l2i", "l2f", "l2d", "f2i", "f2l", "f2d", "d2i", "d2l",
        "d2f", "i2b", "i2c", "i2s", "lcmp", "fcmpl", "fcmpg", "dcmpl",
        "dcmpg", "ifeq", "ifne", "iflt", "ifge", "ifgt", "ifle", "if_icmpeq",
        "if_icmpne", "if_icmplt", "if_icmpge", "if_icmpgt", "if_icmple", "if_acmpeq", "if_acmpne", "goto",
        "jsr", "ret", "tableswitch", "lookupswitch", "ireturn", "lreturn", "freturn", "dreturn",
        "areturn", "return", "getstatic", "putstatic", "getfield", "putfield", "invokevirtual", "invokespecial",
        "invokestatic", "invokeinterface", "invokedynamic", "new", "newarray", "anewarray", "arraylength", "athrow",
        "checkcast", "instanceof", "monitorenter", "monitorexit", "wide", "multianewarray", "ifnull", "ifnonnull",
        "goto_w", "jsr_w", "breakpoint", nullptr // rest are undefined or reserved
    };

    // Inicializa todos como -1 (indefinido) ou preenche com tamanhos conhecidos.
    // O valor aqui é o tamanho dos operandos (quantos bytes vêm DEPOIS do opcode)
    // Então tamanho da instrução = 1 (opcode) + operandSize[opcode].
    int kOperandSize[256] = {0}; // Inicializado com zeros no construtor de escopo global abaixo
    
    // Função para preencher os tamanhos de operandos conhecidos
    void initOperandSizes() {
        for(int i=0; i<256; ++i) kOperandSize[i] = 0; // Maioria é zero
        
        kOperandSize[0x10] = 1; // bipush
        kOperandSize[0x11] = 2; // sipush
        kOperandSize[0x12] = 1; // ldc
        kOperandSize[0x13] = 2; // ldc_w
        kOperandSize[0x14] = 2; // ldc2_w
        kOperandSize[0x15] = 1; // iload
        kOperandSize[0x16] = 1; // lload
        kOperandSize[0x17] = 1; // fload
        kOperandSize[0x18] = 1; // dload
        kOperandSize[0x19] = 1; // aload
        kOperandSize[0x36] = 1; // istore
        kOperandSize[0x37] = 1; // lstore
        kOperandSize[0x38] = 1; // fstore
        kOperandSize[0x39] = 1; // dstore
        kOperandSize[0x3A] = 1; // astore
        kOperandSize[0x84] = 2; // iinc
        kOperandSize[0x99] = 2; // ifeq
        kOperandSize[0x9A] = 2; // ifne
        kOperandSize[0x9B] = 2; // iflt
        kOperandSize[0x9C] = 2; // ifge
        kOperandSize[0x9D] = 2; // ifgt
        kOperandSize[0x9E] = 2; // ifle
        kOperandSize[0x9F] = 2; // if_icmpeq
        kOperandSize[0xA0] = 2; // if_icmpne
        kOperandSize[0xA1] = 2; // if_icmplt
        kOperandSize[0xA2] = 2; // if_icmpge
        kOperandSize[0xA3] = 2; // if_icmpgt
        kOperandSize[0xA4] = 2; // if_icmple
        kOperandSize[0xA5] = 2; // if_acmpeq
        kOperandSize[0xA6] = 2; // if_acmpne
        kOperandSize[0xA7] = 2; // goto
        kOperandSize[0xA8] = 2; // jsr
        kOperandSize[0xA9] = 1; // ret
        kOperandSize[0xB2] = 2; // getstatic
        kOperandSize[0xB3] = 2; // putstatic
        kOperandSize[0xB4] = 2; // getfield
        kOperandSize[0xB5] = 2; // putfield
        kOperandSize[0xB6] = 2; // invokevirtual
        kOperandSize[0xB7] = 2; // invokespecial
        kOperandSize[0xB8] = 2; // invokestatic
        kOperandSize[0xB9] = 4; // invokeinterface
        kOperandSize[0xBA] = 4; // invokedynamic
        kOperandSize[0xBB] = 2; // new
        kOperandSize[0xBC] = 1; // newarray
        kOperandSize[0xBD] = 2; // anewarray
        kOperandSize[0xC0] = 2; // checkcast
        kOperandSize[0xC1] = 2; // instanceof
        kOperandSize[0xC5] = 3; // multianewarray
        kOperandSize[0xC6] = 2; // ifnull
        kOperandSize[0xC7] = 2; // ifnonnull
        kOperandSize[0xC8] = 4; // goto_w
        kOperandSize[0xC9] = 4; // jsr_w
        
        // 0xAA (tableswitch), 0xAB (lookupswitch) e 0xC4 (wide) são variáveis,
        // definimos o tamanho base como -1 para identificá-los mais facilmente.
        kOperandSize[0xAA] = -1;
        kOperandSize[0xAB] = -1;
        kOperandSize[0xC4] = -1;
    }

    struct InitHelper {
        InitHelper() { initOperandSizes(); }
    } initHelper;

    // Helper para ler u2 ou u4 (big-endian) do array de bytes
    unsigned int readOperand(const std::vector<u1> &code, std::size_t offset, int bytes) {
        unsigned int val = 0;
        for (int i = 0; i < bytes; ++i) {
            val = (val << 8) | code[offset + i];
        }
        return val;
    }
}

std::string BytecodeDecoder::mnemonic(u1 opcode) const {
    if (kMnemonics[opcode] != nullptr) {
        return kMnemonics[opcode];
    }
    std::ostringstream output;
    output << "op_0x" << std::hex << std::uppercase << static_cast<unsigned int>(opcode);
    return output.str();
}

std::vector<Instruction> BytecodeDecoder::decode(const std::vector<u1> &code) const {
    std::vector<Instruction> instructions;
    std::size_t pc = 0;

    while (pc < code.size()) {
        Instruction instr;
        instr.pc = pc;
        instr.opcode = code[pc];
        instr.mnemonic = mnemonic(instr.opcode);

        int op_size = kOperandSize[instr.opcode];

        // Lida com casos normais e operandos (CP references, constantes, saltos)
        if (op_size > 0 && (pc + 1 + op_size <= code.size())) {
            unsigned int operand_val = readOperand(code, pc + 1, op_size);
            
            // Para instruções que acessam a Constant Pool, usamos #
            // Outras como ldc usam #1, ldc_w usa #2, field/methods usam #2
            if (instr.opcode == 0x12 || (instr.opcode >= 0x13 && instr.opcode <= 0x14) || 
                (instr.opcode >= 0xB2 && instr.opcode <= 0xBA) || instr.opcode == 0xBB || 
                instr.opcode == 0xBD || instr.opcode == 0xC0 || instr.opcode == 0xC1) {
                instr.operand_text = "#" + std::to_string(operand_val);
            } 
            // Para branches (gotos, if), os bytes são offsets do PC atual.
            // Formato igual ao jclasslib: <alvo> (<+|-offset>)
            else if ((instr.opcode >= 0x99 && instr.opcode <= 0xA8) || instr.opcode == 0xC6 || instr.opcode == 0xC7) {
                short offset = static_cast<short>(operand_val);
                int target = pc + offset;
                std::string sign = offset >= 0 ? "+" : "";
                instr.operand_text = std::to_string(target) + " (" + sign + std::to_string(offset) + ")";
            }
            else if (instr.opcode == 0x84) { // iinc
                unsigned int index = code[pc + 1];
                int const_val = static_cast<int>(static_cast<signed char>(code[pc + 2]));
                instr.operand_text = std::to_string(index) + " by " + std::to_string(const_val);
            }
            else if (instr.opcode == 0xBC) { // newarray
                std::string type_name = "unknown";
                switch (operand_val) {
                    case 4: type_name = "boolean"; break;
                    case 5: type_name = "char"; break;
                    case 6: type_name = "float"; break;
                    case 7: type_name = "double"; break;
                    case 8: type_name = "byte"; break;
                    case 9: type_name = "short"; break;
                    case 10: type_name = "int"; break;
                    case 11: type_name = "long"; break;
                }
                instr.operand_text = std::to_string(operand_val) + " (" + type_name + ")";
            }
            else {
                // Outros (bipush, local variables index)
                instr.operand_text = std::to_string(operand_val);
            }
            instr.length = 1 + op_size;
        } 
        // Casos Especiais (Comprimento Variável)
        else if (op_size == -1) {
            if (instr.opcode == 0xC4) { // wide
                if (pc + 1 < code.size()) {
                    u1 wide_opcode = code[pc + 1];
                    if (wide_opcode == 0x84) { // iinc
                        instr.operand_text = std::to_string(readOperand(code, pc + 2, 2)) + ", " + std::to_string(readOperand(code, pc + 4, 2));
                        instr.length = 6;
                    } else { // aload, iload, etc
                        instr.operand_text = std::to_string(readOperand(code, pc + 2, 2));
                        instr.length = 4;
                    }
                }
            } else if (instr.opcode == 0xAA) { // tableswitch
                std::size_t offset = pc + 1;
                // Pular padding (alinhamento para múltiplos de 4 bytes)
                while (offset % 4 != 0) offset++;
                
                unsigned int default_jmp = readOperand(code, offset, 4); offset += 4;
                int low = static_cast<int>(readOperand(code, offset, 4)); offset += 4;
                int high = static_cast<int>(readOperand(code, offset, 4)); offset += 4;
                
                instr.operand_text = "[low=" + std::to_string(low) + ", high=" + std::to_string(high) + ", default=" + std::to_string(pc + default_jmp) + "]";
                instr.length = offset - pc + (high - low + 1) * 4;
            } else if (instr.opcode == 0xAB) { // lookupswitch
                std::size_t offset = pc + 1;
                // Pular padding
                while (offset % 4 != 0) offset++;
                
                unsigned int default_jmp = readOperand(code, offset, 4); offset += 4;
                unsigned int npairs = readOperand(code, offset, 4); offset += 4;
                
                instr.operand_text = "[npairs=" + std::to_string(npairs) + ", default=" + std::to_string(pc + default_jmp) + "]";
                instr.length = offset - pc + npairs * 8;
            }
        } 
        else {
            // Instrução sem operandos (tamanho 1 byte)
            instr.length = 1;
        }

        instructions.push_back(instr);
        pc += instr.length;
    }

    return instructions;
}
