#include "bytecode_viewer/ClassReader.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace {

// Lê uma entrada individual da constant pool seguindo a ordem definida pela JVM.
CpInfo parseNextConstantPoolEntry(std::istream &input) {
    CpInfo entry;
    entry.tag = readU1(input);

    switch (entry.tag) {
        case 1: { // CONSTANT_Utf8
            const u2 utf8_length = readU2(input);
            const std::vector<u1> bytes = readBytes(input, utf8_length);
            entry.utf8_value = std::string(bytes.begin(), bytes.end());
            break;
        }
        case 7:   // CONSTANT_Class
        case 8:   // CONSTANT_String
        case 16:  // CONSTANT_MethodType
            entry.index1 = readU2(input);
            break;
        case 3:   // CONSTANT_Integer
        case 4:   // CONSTANT_Float
            entry.bytes = readBytes(input, 4);
            break;
        case 5:   // CONSTANT_Long
        case 6:   // CONSTANT_Double
            entry.bytes = readBytes(input, 8);
            break;
        case 9:   // CONSTANT_Fieldref
        case 10:  // CONSTANT_Methodref
        case 11:  // CONSTANT_InterfaceMethodref
        case 12:  // CONSTANT_NameAndType
        case 18:  // CONSTANT_InvokeDynamic
            entry.index1 = readU2(input);
            entry.index2 = readU2(input);
            break;
        case 15:  // CONSTANT_MethodHandle
            entry.index1 = readU1(input);
            entry.index2 = readU2(input);
            break;
        default:
            throw std::runtime_error("Unknown Constant Pool tag: " + std::to_string(static_cast<int>(entry.tag)));
    }

    return entry;
}

} // namespace

AttributeInfo ClassReader::readAttribute(std::istream &input) {
    AttributeInfo attribute;
    attribute.name_index = readU2(input);
    const u4 attribute_length = readU4(input);
    attribute.info = readBytes(input, attribute_length);
    return attribute;
}

// O fluxo deve estar posicionado logo após o campo attribute_length,
// apontando para o início do payload do atributo Code.
CodeAttribute ClassReader::readCodeAttribute(std::istream &input) {
    CodeAttribute code_attribute;

    code_attribute.max_stack = readU2(input);
    code_attribute.max_locals = readU2(input);

    const u4 code_length = readU4(input);
    code_attribute.code = readBytes(input, code_length);

    const u2 exception_table_length = readU2(input);
    code_attribute.exception_table.reserve(exception_table_length);
    for (u2 index = 0; index < exception_table_length; ++index) {
        ExceptionTableEntry entry;
        entry.start_pc = readU2(input);
        entry.end_pc = readU2(input);
        entry.handler_pc = readU2(input);
        entry.catch_type = readU2(input);
        code_attribute.exception_table.push_back(entry);
    }

    const u2 nested_attribute_count = readU2(input);
    code_attribute.attributes.reserve(nested_attribute_count);
    for (u2 index = 0; index < nested_attribute_count; ++index) {
        code_attribute.attributes.push_back(readAttribute(input));
    }

    return code_attribute;
}

ClassFile ClassReader::read(std::istream &input) const {
    ClassFile class_file;

    class_file.magic = readU4(input);
    if (class_file.magic != 0xCAFEBABE) {
        throw std::runtime_error("Invalid class file: magic number 0xCAFEBABE not found.");
    }

    class_file.minor_version = readU2(input);
    class_file.major_version = readU2(input);

    const u2 constant_pool_count = readU2(input);
    std::vector<CpInfo> constant_pool_entries;
    constant_pool_entries.push_back(CpInfo());

    for (u2 index = 1; index < constant_pool_count; ++index) {
        CpInfo entry = parseNextConstantPoolEntry(input);
        constant_pool_entries.push_back(entry);

        // A JVM reserva dois slots consecutivos para CONSTANT_Long e CONSTANT_Double.
        if (entry.tag == 5 || entry.tag == 6) {
            constant_pool_entries.push_back(CpInfo());
            ++index;
        }
    }
    class_file.constant_pool = ConstantPool(constant_pool_entries);

    class_file.access_flags = readU2(input);
    class_file.this_class = readU2(input);
    class_file.super_class = readU2(input);

    const u2 interfaces_count = readU2(input);
    class_file.interfaces.reserve(interfaces_count);
    for (u2 index = 0; index < interfaces_count; ++index) {
        class_file.interfaces.push_back(readU2(input));
    }

    const u2 fields_count = readU2(input);
    class_file.fields.reserve(fields_count);
    for (u2 index = 0; index < fields_count; ++index) {
        FieldInfo field;
        field.access_flags = readU2(input);
        field.name_index = readU2(input);
        field.descriptor_index = readU2(input);

        const u2 field_attribute_count = readU2(input);
        field.attributes.reserve(field_attribute_count);
        for (u2 attribute_index = 0; attribute_index < field_attribute_count; ++attribute_index) {
            field.attributes.push_back(readAttribute(input));
        }
        class_file.fields.push_back(field);
    }

    const u2 methods_count = readU2(input);
    class_file.methods.reserve(methods_count);
    for (u2 index = 0; index < methods_count; ++index) {
        MethodInfo method;
        method.access_flags = readU2(input);
        method.name_index = readU2(input);
        method.descriptor_index = readU2(input);
        method.has_code = false;

        const u2 method_attribute_count = readU2(input);
        method.attributes.reserve(method_attribute_count);
        for (u2 attribute_index = 0; attribute_index < method_attribute_count; ++attribute_index) {
            AttributeInfo attribute = readAttribute(input);
            const std::string attribute_name = class_file.constant_pool.utf8(attribute.name_index);

            if (attribute_name == "Code") {
                std::string raw_data(attribute.info.begin(), attribute.info.end());
                std::istringstream code_input(raw_data);
                method.code = readCodeAttribute(code_input);
                method.has_code = true;
            } else {
                method.attributes.push_back(attribute);
            }
        }
        class_file.methods.push_back(method);
    }

    const u2 class_attribute_count = readU2(input);
    class_file.attributes.reserve(class_attribute_count);
    for (u2 index = 0; index < class_attribute_count; ++index) {
        class_file.attributes.push_back(readAttribute(input));
    }

    return class_file;
}

ClassFile ClassReader::readFile(const std::string &path) const {
    std::ifstream input(path.c_str(), std::ios::binary);
    if (!input) {
        throw std::runtime_error("unable to open class file: " + path);
    }
    return read(input);
}

std::string ClassFile::thisClassName() const {
    if (this_class == 0) {
        return std::string();
    }
    return constant_pool.className(this_class);
}

std::string ClassFile::superClassName() const {
    if (super_class == 0) {
        return std::string();
    }
    return constant_pool.className(super_class);
}