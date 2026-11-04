#include "bytecode_viewer/BytecodeDecoder.hpp"

#include <iomanip>
#include <sstream>

namespace {

// Tabela de mnemônicos da JVM. O índice corresponde ao opcode em hexadecimal.
const char *kJavaMnemonics[256] = {
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
    "goto_w", "jsr_w", "breakpoint", nullptr
};

// Quantidade de bytes de operandos que cada opcode exige.
// Valor -1 indica instrução com tamanho variável.
int kOperandSizes[256] = {0};

void initializeOperandSizes() {
    for (int index = 0; index < 256; ++index) {
        kOperandSizes[index] = 0;
    }

    kOperandSizes[0x10] = 1;  // bipush
    kOperandSizes[0x11] = 2;  // sipush
    kOperandSizes[0x12] = 1;  // ldc
    kOperandSizes[0x13] = 2;  // ldc_w
    kOperandSizes[0x14] = 2;  // ldc2_w
    kOperandSizes[0x15] = 1;  // iload
    kOperandSizes[0x16] = 1;  // lload
    kOperandSizes[0x17] = 1;  // fload
    kOperandSizes[0x18] = 1;  // dload
    kOperandSizes[0x19] = 1;  // aload
    kOperandSizes[0x36] = 1;  // istore
    kOperandSizes[0x37] = 1;  // lstore
    kOperandSizes[0x38] = 1;  // fstore
    kOperandSizes[0x39] = 1;  // dstore
    kOperandSizes[0x3A] = 1;  // astore
    kOperandSizes[0x84] = 2;  // iinc
    kOperandSizes[0x99] = 2;  // ifeq
    kOperandSizes[0x9A] = 2;  // ifne
    kOperandSizes[0x9B] = 2;  // iflt
    kOperandSizes[0x9C] = 2;  // ifge
    kOperandSizes[0x9D] = 2;  // ifgt
    kOperandSizes[0x9E] = 2;  // ifle
    kOperandSizes[0x9F] = 2;  // if_icmpeq
    kOperandSizes[0xA0] = 2;  // if_icmpne
    kOperandSizes[0xA1] = 2;  // if_icmplt
    kOperandSizes[0xA2] = 2;  // if_icmpge
    kOperandSizes[0xA3] = 2;  // if_icmpgt
    kOperandSizes[0xA4] = 2;  // if_icmple
    kOperandSizes[0xA5] = 2;  // if_acmpeq
    kOperandSizes[0xA6] = 2;  // if_acmpne
    kOperandSizes[0xA7] = 2;  // goto
    kOperandSizes[0xA8] = 2;  // jsr
    kOperandSizes[0xA9] = 1;  // ret
    kOperandSizes[0xB2] = 2;  // getstatic
    kOperandSizes[0xB3] = 2;  // putstatic
    kOperandSizes[0xB4] = 2;  // getfield
    kOperandSizes[0xB5] = 2;  // putfield
    kOperandSizes[0xB6] = 2;  // invokevirtual
    kOperandSizes[0xB7] = 2;  // invokespecial
    kOperandSizes[0xB8] = 2;  // invokestatic
    kOperandSizes[0xB9] = 4;  // invokeinterface
    kOperandSizes[0xBA] = 4;  // invokedynamic
    kOperandSizes[0xBB] = 2;  // new
    kOperandSizes[0xBC] = 1;  // newarray
    kOperandSizes[0xBD] = 2;  // anewarray
    kOperandSizes[0xC0] = 2;  // checkcast
    kOperandSizes[0xC1] = 2;  // instanceof
    kOperandSizes[0xC5] = 3;  // multianewarray
    kOperandSizes[0xC6] = 2;  // ifnull
    kOperandSizes[0xC7] = 2;  // ifnonnull
    kOperandSizes[0xC8] = 4;  // goto_w
    kOperandSizes[0xC9] = 4;  // jsr_w

    // Instruções variáveis: tableswitch, lookupswitch e wide.
    kOperandSizes[0xAA] = -1;
    kOperandSizes[0xAB] = -1;
    kOperandSizes[0xC4] = -1;
}

struct OperandSizeInitializer {
    OperandSizeInitializer() {
        initializeOperandSizes();
    }
};

OperandSizeInitializer g_operand_size_initializer;

// Lê um valor de `bytes` bytes em big-endian a partir do buffer de código.
unsigned int readOperandValue(const std::vector<u1> &code, std::size_t offset, int bytes) {
    unsigned int value = 0;
    for (int index = 0; index < bytes; ++index) {
        value = (value << 8) | code[offset + index];
    }
    return value;
}

bool isConstantPoolReferenceOpcode(u1 opcode) {
    return opcode == 0x12 ||
           (opcode >= 0x13 && opcode <= 0x14) ||
           (opcode >= 0xB2 && opcode <= 0xBA) ||
           opcode == 0xBB || opcode == 0xBD ||
           opcode == 0xC0 || opcode == 0xC1;
}

bool isBranchOpcode(u1 opcode) {
    return (opcode >= 0x99 && opcode <= 0xA8) || opcode == 0xC6 || opcode == 0xC7;
}

std::string formatNewArrayDescriptor(unsigned int descriptor_code) {
    switch (descriptor_code) {
        case 4:  return "boolean";
        case 5:  return "char";
        case 6:  return "float";
        case 7:  return "double";
        case 8:  return "byte";
        case 9:  return "short";
        case 10: return "int";
        case 11: return "long";
        default: return "unknown";
    }
}

} // namespace

std::string BytecodeDecoder::mnemonic(u1 opcode) const {
    if (kJavaMnemonics[opcode] != nullptr) {
        return kJavaMnemonics[opcode];
    }

    std::ostringstream output;
    output << "op_0x" << std::hex << std::uppercase
           << static_cast<unsigned int>(opcode);
    return output.str();
}

std::vector<Instruction> BytecodeDecoder::decode(const std::vector<u1> &code) const {
    std::vector<Instruction> instructions;
    std::size_t program_counter = 0;

    while (program_counter < code.size()) {
        Instruction instruction;
        instruction.pc = program_counter;
        instruction.opcode = code[program_counter];
        instruction.mnemonic = mnemonic(instruction.opcode);

        const int operand_size = kOperandSizes[instruction.opcode];

        if (operand_size > 0 && (program_counter + 1 + operand_size <= code.size())) {
            const unsigned int operand_value = readOperandValue(code, program_counter + 1, operand_size);

            if (isConstantPoolReferenceOpcode(instruction.opcode)) {
                instruction.operand_text = "#" + std::to_string(operand_value);
            } else if (isBranchOpcode(instruction.opcode)) {
                const short branch_offset = static_cast<short>(operand_value);
                const int target_pc = static_cast<int>(program_counter) + branch_offset;
                const std::string sign = branch_offset >= 0 ? "+" : "";
                instruction.operand_text = std::to_string(target_pc) + " (" + sign + std::to_string(branch_offset) + ")";
            } else if (instruction.opcode == 0x84) {
                const unsigned int local_index = code[program_counter + 1];
                const int increment = static_cast<int>(static_cast<signed char>(code[program_counter + 2]));
                instruction.operand_text = std::to_string(local_index) + " by " + std::to_string(increment);
            } else if (instruction.opcode == 0xBC) {
                const std::string array_type = formatNewArrayDescriptor(operand_value);
                instruction.operand_text = std::to_string(operand_value) + " (" + array_type + ")";
            } else {
                instruction.operand_text = std::to_string(operand_value);
            }

            instruction.length = 1u + static_cast<std::size_t>(operand_size);
        } else if (operand_size == -1) {
            if (instruction.opcode == 0xC4) {
                if (program_counter + 1 < code.size()) {
                    const u1 wide_opcode = code[program_counter + 1];
                    if (wide_opcode == 0x84) {
                        const unsigned int local_index = readOperandValue(code, program_counter + 2, 2);
                        const unsigned int increment = readOperandValue(code, program_counter + 4, 2);
                        instruction.operand_text = std::to_string(local_index) + ", " + std::to_string(increment);
                        instruction.length = 6;
                    } else {
                        instruction.operand_text = std::to_string(readOperandValue(code, program_counter + 2, 2));
                        instruction.length = 4;
                    }
                }
            } else if (instruction.opcode == 0xAA) {
                std::size_t offset = program_counter + 1;
                while (offset % 4 != 0) {
                    ++offset;
                }

                const unsigned int default_jump = readOperandValue(code, offset, 4);
                offset += 4;
                const int low = static_cast<int>(readOperandValue(code, offset, 4));
                offset += 4;
                const int high = static_cast<int>(readOperandValue(code, offset, 4));
                offset += 4;

                instruction.operand_text = "[low=" + std::to_string(low) + ", high=" + std::to_string(high)
                    + ", default=" + std::to_string(program_counter + default_jump) + "]";
                instruction.length = offset - program_counter + static_cast<std::size_t>(high - low + 1) * 4;
            } else if (instruction.opcode == 0xAB) {
                std::size_t offset = program_counter + 1;
                while (offset % 4 != 0) {
                    ++offset;
                }

                const unsigned int default_jump = readOperandValue(code, offset, 4);
                offset += 4;
                const unsigned int pair_count = readOperandValue(code, offset, 4);
                offset += 4;

                instruction.operand_text = "[npairs=" + std::to_string(pair_count)
                    + ", default=" + std::to_string(program_counter + default_jump) + "]";
                instruction.length = offset - program_counter + pair_count * 8;
            }
        } else {
            instruction.length = 1;
        }

        instructions.push_back(instruction);
        program_counter += instruction.length;
    }

    return instructions;
}
