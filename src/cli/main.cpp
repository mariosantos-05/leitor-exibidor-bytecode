/*
 * main.cpp
 * Aplicação CLI para ler um arquivo .class e exibir informações do ClassFile,
 * do pool de constantes e do bytecode decodificado.
 */

#include "bytecode_viewer/BytecodeDecoder.hpp"
#include "bytecode_viewer/ClassReader.hpp"

#include <cctype>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace {

std::string describeJavaVersion(u2 major_version) {
    static const std::pair<u2, const char *> kVersionMap[] = {
        {45, "1.1"}, {46, "1.2"}, {47, "1.3"}, {48, "1.4"}, {49, "5.0"},
        {50, "6"},   {51, "7"},   {52, "8"},   {53, "9"},   {54, "10"},
        {55, "11"},  {56, "12"},  {57, "13"},  {58, "14"},  {59, "15"},
        {60, "16"},  {61, "17"},  {62, "18"},  {63, "19"},  {64, "20"},
        {65, "21"},
    };
    for (const auto &entry : kVersionMap) {
        if (entry.first == major_version) {
            return std::string("[") + entry.second + "]";
        }
    }
    return "[" + std::to_string(major_version) + "]";
}

std::string describeAccessFlags(u2 flags) {
    static const std::pair<u2, const char *> kFlagMap[] = {
        {0x0001, "public"},     {0x0010, "final"},      {0x0020, "super"},
        {0x0200, "interface"},  {0x0400, "abstract"},   {0x1000, "synthetic"},
        {0x2000, "annotation"}, {0x4000, "enum"},
    };
    std::ostringstream output;
    output << "0x" << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << flags << std::dec << std::setfill(' ');
    output << " [";
    bool is_first = true;
    for (const auto &entry : kFlagMap) {
        if (flags & entry.first) {
            if (!is_first) output << ", ";
            output << entry.second;
            is_first = false;
        }
    }
    output << "]";
    return output.str();
}

// Resolve operandos que apontam para a constant pool, como "#12",
// para uma representação mais amigável em tela.
std::string resolveConstantOperandText(const std::string &operand_text, const ConstantPool &constant_pool) {
    if (operand_text.empty() || operand_text[0] != '#') {
        return operand_text;
    }

    std::size_t cursor = 1;
    while (cursor < operand_text.size() && std::isdigit(static_cast<unsigned char>(operand_text[cursor]))) {
        ++cursor;
    }

    if (cursor == 1) {
        return operand_text;
    }

    std::istringstream input(operand_text.substr(1, cursor - 1));
    unsigned int index = 0;
    input >> index;
    if (!input) {
        return operand_text;
    }

    std::string resolved = constant_pool.toString(static_cast<u2>(index));

    std::size_t comment_position = resolved.find("// ");
    if (comment_position != std::string::npos) {
        resolved = resolved.substr(comment_position + 3);
        std::size_t colon_position = resolved.find(":");
        if (colon_position != std::string::npos && colon_position > 0 && resolved[colon_position - 1] != ' ') {
            resolved = resolved.substr(0, colon_position) + " : " + resolved.substr(colon_position + 1);
        }
    } else if (resolved.find("Utf8") != std::string::npos) {
        std::size_t quote_position = resolved.find("\"");
        if (quote_position != std::string::npos) {
            resolved = resolved.substr(quote_position + 1);
            if (!resolved.empty() && resolved.back() == '"') {
                resolved.pop_back();
            }
        }
    }

    return std::string(" #") + operand_text.substr(1, cursor - 1) + " <" + resolved + ">" + operand_text.substr(cursor);
}

void printClassFileOverview(const ClassFile &class_file) {
    constexpr int kLabelWidth = 25;
    std::cout << "General Information:\n";
    std::cout << "  " << std::left << std::setw(kLabelWidth) << "Magic:"
              << "0x" << std::hex << std::uppercase << std::setw(8) << std::setfill('0')
              << class_file.magic << std::dec << std::setfill(' ') << '\n';
    std::cout << "  " << std::left << std::setw(kLabelWidth) << "Minor version:" << class_file.minor_version << '\n';
    std::cout << "  " << std::left << std::setw(kLabelWidth) << "Major version:"
              << class_file.major_version << " " << describeJavaVersion(class_file.major_version) << '\n';
    std::cout << "  " << std::left << std::setw(kLabelWidth) << "Constant pool count:" << class_file.constant_pool.size() << '\n';
    std::cout << "  " << std::left << std::setw(kLabelWidth) << "Access flags:" << describeAccessFlags(class_file.access_flags) << '\n';

    std::string this_class_text = "cp_info #" + std::to_string(class_file.this_class)
                                 + " <" + class_file.thisClassName() + ">";
    std::cout << "  " << std::left << std::setw(kLabelWidth) << "This class:" << this_class_text << '\n';

    std::string super_class_text = "cp_info #" + std::to_string(class_file.super_class);
    if (class_file.super_class != 0) {
        super_class_text += " <" + class_file.superClassName() + ">";
    }
    std::cout << "  " << std::left << std::setw(kLabelWidth) << "Super class:" << super_class_text << '\n';

    std::cout << "  " << std::left << std::setw(kLabelWidth) << "Interfaces count:" << class_file.interfaces.size() << '\n';
    std::cout << "  " << std::left << std::setw(kLabelWidth) << "Fields count:" << class_file.fields.size() << '\n';
    std::cout << "  " << std::left << std::setw(kLabelWidth) << "Methods count:" << class_file.methods.size() << '\n';
    std::cout << "  " << std::left << std::setw(kLabelWidth) << "Attributes count:" << class_file.attributes.size() << '\n';
}

void printDetailedClassFile(const ClassFile &class_file) {
    printClassFileOverview(class_file);

    std::cout << "\nConstant Pool:\n";
    for (u2 index = 1; index <= class_file.constant_pool.size(); ++index) {
        std::cout << "  #" << index << " = " << class_file.constant_pool.toString(index) << '\n';
    }

    std::cout << "\nMethods:\n";
    BytecodeDecoder decoder;
    for (const MethodInfo &method : class_file.methods) {
        std::cout << "  Method Name: " << class_file.constant_pool.utf8(method.name_index) << '\n';
        std::cout << "  Descriptor:  " << class_file.constant_pool.utf8(method.descriptor_index) << '\n';
        if (method.has_code) {
            std::cout << "  Code Attribute:\n";
            for (const Instruction &instruction : decoder.decode(method.code.code)) {
                std::cout << std::right << std::setw(4) << instruction.pc << " "
                          << std::left << std::setw(15) << instruction.mnemonic;
                if (!instruction.operand_text.empty()) {
                    std::string operand_text = instruction.operand_text;
                    if (operand_text[0] == '#') {
                        std::cout << resolveConstantOperandText(operand_text, class_file.constant_pool);
                    } else {
                        std::cout << " " << operand_text;
                    }
                }
                std::cout << '\n';
            }
        }
        std::cout << '\n';
    }
}

} // namespace

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <class-file>\n";
        return 1;
    }

    try {
        ClassReader reader;
        const ClassFile class_file = reader.readFile(argv[1]);
        printDetailedClassFile(class_file);
    } catch (const std::exception &error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
