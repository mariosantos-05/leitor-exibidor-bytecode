#pragma once

#include "bytecode_viewer/Common.hpp"

#include <string>
#include <vector>

/*
 * ConstantPool.hpp
 * The constant pool is the class file's symbol table. It stores literals,
 * type names, method and field references, and other values used by the
 * JVM. Each entry has a `tag` identifying its type (e.g., CONSTANT_Utf8=1,
 * CONSTANT_Class=7).
 */

// A single constant pool entry. Fields are generic and only some are used
// depending on the `tag` value.
struct CpInfo {
    u1 tag;                // entry type tag (see JVM Spec constant tags)
    std::string utf8_value; // used when tag == CONSTANT_Utf8
    u2 index1;             // operand index (meaning depends on tag)
    u2 index2;             // second operand index (if needed)
    std::vector<u1> bytes; // raw bytes for some constant types

    CpInfo()
        : tag(0), index1(0), index2(0) {}
};

// A small helper class to query the constant pool. The implementation
// provides convenience methods to resolve common patterns (utf8, class
// names, name-and-type pairs) and to pretty-print entries for debugging.
class ConstantPool {
public:
    ConstantPool();
    explicit ConstantPool(std::vector<CpInfo> entries);

    // Number of entries in the pool
    std::size_t size() const;
    bool empty() const;

    // Access raw CpInfo at 1-based constant pool index (per JVM spec)
    const CpInfo &at(u2 index) const;

    // Helpers to resolve common constant pool contents
    std::string utf8(u2 index) const;
    std::string className(u2 index) const;
    std::string nameAndType(u2 index) const;
    std::string toString(u2 index) const;

private:
    std::vector<CpInfo> entries_;
};