#pragma once

#include "bytecode_viewer/Common.hpp"

#include <vector>

/*
 * ClassStructures.hpp
 * Definitions of the in-memory structures that mirror parts of the JVM
 * ClassFile format (see JVM Spec §4.1 - The ClassFile Structure).
 * These are plain data structs intended to be easy to read and understand
 * for learners.
 */

// AttributeInfo: a generic attribute with a name index into the constant pool
// and a raw byte payload. Attributes are used in many places inside a
// ClassFile (fields, methods, class-level attributes).
struct AttributeInfo {
    u2 name_index;           // index into the constant pool for the attribute name
    std::vector<u1> info;    // raw attribute bytes (format depends on attribute)

    AttributeInfo()
        : name_index(0) {}
};

// ExceptionTableEntry: used inside the Code attribute to describe exception
// handlers. Each entry specifies a try range and where to jump on exception.
struct ExceptionTableEntry {
    u2 start_pc;
    u2 end_pc;
    u2 handler_pc;
    u2 catch_type; // constant pool index for the exception class, 0 = finally

    ExceptionTableEntry()
        : start_pc(0), end_pc(0), handler_pc(0), catch_type(0) {}
};

// CodeAttribute: contains the actual bytecode for a method plus supporting
// data such as max stack size and exception table.
struct CodeAttribute {
    u2 max_stack;                          // maximum operand stack depth used by this method
    u2 max_locals;                         // number of local variable slots
    std::vector<u1> code;                  // the bytecode instructions
    std::vector<ExceptionTableEntry> exception_table; // exception handlers
    std::vector<AttributeInfo> attributes; // nested attributes for the code

    CodeAttribute()
        : max_stack(0), max_locals(0) {}
};

// FieldInfo: metadata for a field in a class (name, descriptor, access flags)
struct FieldInfo {
    u2 access_flags;                       // modifiers (public/private/static/...)
    u2 name_index;                         // constant pool index for the field name
    u2 descriptor_index;                   // constant pool index for the field descriptor
    std::vector<AttributeInfo> attributes; // field attributes (e.g., ConstantValue)

    FieldInfo()
        : access_flags(0), name_index(0), descriptor_index(0) {}
};

// MethodInfo: metadata for a method. If the method has code, the `has_code`
// flag is true and `code` holds the CodeAttribute parsed from the class file.
struct MethodInfo {
    u2 access_flags;
    u2 name_index;
    u2 descriptor_index;
    bool has_code;
    CodeAttribute code;                    // code attribute (if present)
    std::vector<AttributeInfo> attributes; // other method-level attributes

    MethodInfo()
        : access_flags(0), name_index(0), descriptor_index(0), has_code(false) {}
};