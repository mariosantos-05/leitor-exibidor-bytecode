#include "bytecode_viewer/ConstantPool.hpp"

#include <stdexcept>
#include <utility>

ConstantPool::ConstantPool()
    : entries_(1) {}

ConstantPool::ConstantPool(std::vector<CpInfo> entries)
    : entries_(std::move(entries)) {
    if (entries_.empty()) {
        entries_.push_back(CpInfo());
    }
}

std::size_t ConstantPool::size() const {
    return entries_.size() > 0 ? entries_.size() - 1 : 0;
}

bool ConstantPool::empty() const {
    return size() == 0;
}

const CpInfo &ConstantPool::at(u2 index) const {
    if (index == 0 || index >= entries_.size()) {
        throw std::out_of_range("constant pool index out of range: " + std::to_string(index));
    }
    return entries_.at(index);
}

// Resolve uma string UTF-8 direta da constant pool.
std::string ConstantPool::utf8(u2 index) const {
    const CpInfo &entry = at(index);
    if (entry.tag != 1) { // 1 = CONSTANT_Utf8
        return "<invalid-utf8-tag>";
    }
    return entry.utf8_value;
}

// CONSTANT_Class aponta para uma entrada Utf8 com o nome da classe.
std::string ConstantPool::className(u2 index) const {
    const CpInfo &entry = at(index);
    if (entry.tag != 7) { // 7 = CONSTANT_Class
        return "<invalid-class-tag>";
    }
    return utf8(entry.index1);
}

// NameAndType combina nome e descritor em um formato `nome:descritor`.
std::string ConstantPool::nameAndType(u2 index) const {
    const CpInfo &entry = at(index);
    if (entry.tag != 12) { // 12 = CONSTANT_NameAndType
        return "<invalid-nameandtype-tag>";
    }
    return utf8(entry.index1) + ":" + utf8(entry.index2);
}

// Formata uma entrada da constant pool para visualização amigável.
std::string ConstantPool::toString(u2 index) const {
    if (index == 0 || index >= entries_.size()) {
        return "null";
    }

    const CpInfo &entry = entries_.at(index);

    switch (entry.tag) {
        case 1:  return "Utf8               \"" + entry.utf8_value + "\"";
        case 7:  return "Class              #" + std::to_string(entry.index1) + " // " + className(index);
        case 8:  return "String             #" + std::to_string(entry.index1) + " // " + utf8(entry.index1);
        case 9:  return "Fieldref           #" + std::to_string(entry.index1) + ".#" + std::to_string(entry.index2)
            + " // " + className(entry.index1) + "." + nameAndType(entry.index2);
        case 10: return "Methodref          #" + std::to_string(entry.index1) + ".#" + std::to_string(entry.index2)
            + " // " + className(entry.index1) + "." + nameAndType(entry.index2);
        case 11: return "InterfaceMethodref #" + std::to_string(entry.index1) + ".#" + std::to_string(entry.index2)
            + " // " + className(entry.index1) + "." + nameAndType(entry.index2);
        case 12: return "NameAndType        #" + std::to_string(entry.index1) + ":#" + std::to_string(entry.index2)
            + " // " + nameAndType(index);
        case 3:  return "Integer";
        case 4:  return "Float";
        case 5:  return "Long";
        case 6:  return "Double";
        default: return "UnknownTag         " + std::to_string(static_cast<int>(entry.tag));
    }
}