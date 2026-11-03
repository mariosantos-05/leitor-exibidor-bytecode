#pragma once

#include "bytecode_viewer/ClassStructures.hpp"
#include "bytecode_viewer/ConstantPool.hpp"

#include <istream>
#include <string>
#include <vector>

/*
 * ClassReader.hpp
 * High-level parser for a JVM ClassFile. It reads the binary class file
 * format (see JVM Spec §4.1) and populates the `ClassFile` struct defined
 * in ClassStructures.hpp.
 */

// Representation of a parsed class file. Fields correspond to entries in
// the ClassFile structure from the JVM Specification.
struct ClassFile {
    u4 magic;                   // should be 0xCAFEBABE
    u2 minor_version;           // class file minor version
    u2 major_version;           // class file major version
    ConstantPool constant_pool; // constant pool (symbol table)
    u2 access_flags;            // class modifiers (public, final, etc.)
    u2 this_class;              // constant pool index of this class
    u2 super_class;             // constant pool index of super class
    std::vector<u2> interfaces; // list of implemented interfaces (cp indices)
    std::vector<FieldInfo> fields;
    std::vector<MethodInfo> methods;
    std::vector<AttributeInfo> attributes;

    ClassFile()
        : magic(0), minor_version(0), major_version(0), access_flags(0), this_class(0), super_class(0) {}

    // Helper methods to get human-readable class names
    std::string thisClassName() const;
    std::string superClassName() const;
};

// ClassReader: provides methods to read a ClassFile from a stream or file
// path. Implementations throw std::runtime_error on parse errors.
class ClassReader {
public:
    // Parse a ClassFile from an input stream (binary mode expected).
    ClassFile read(std::istream &input) const;

    // Convenience: open file at `path` and parse it.
    ClassFile readFile(const std::string &path) const;

private:
    // Internal helpers for reading nested structures
    static AttributeInfo readAttribute(std::istream &input);
    static CodeAttribute readCodeAttribute(std::istream &input);
};