#pragma once

#include <cstddef>
#include <cstdint>
#include <istream>
#include <stdexcept>
#include <string>
#include <vector>

/*
 * Common.hpp
 * Small utilities for reading JVM class file binary data.
 *
 * The JVM class file format uses big-endian encoding and fixed-size
 * integer types. We provide lightweight aliases and helper functions
 * so the rest of the code can read the binary format clearly.
 *
 * Types:
 *  - u1: unsigned 8-bit value (Java "byte")
 *  - u2: unsigned 16-bit value (Java "unsigned short" concept)
 *  - u4: unsigned 32-bit value (Java "unsigned int" concept)
 */

using u1 = std::uint8_t;
using u2 = std::uint16_t;
using u4 = std::uint32_t;

// Read one unsigned byte (u1) from the stream.
// Throws std::runtime_error if the stream ends unexpectedly.
inline u1 readU1(std::istream &input) {
    char value = 0;
    if (!input.get(value)) {
        throw std::runtime_error("unexpected end of stream while reading u1");
    }
    return static_cast<u1>(static_cast<unsigned char>(value));
}

// Read two bytes (big-endian) and return as u2.
inline u2 readU2(std::istream &input) {
    return static_cast<u2>((static_cast<u2>(readU1(input)) << 8) |
                           static_cast<u2>(readU1(input)));
}

// Read four bytes (big-endian) and return as u4.
inline u4 readU4(std::istream &input) {
    return static_cast<u4>((static_cast<u4>(readU2(input)) << 16) |
                           static_cast<u4>(readU2(input)));
}

// Read a raw byte array of length `count` from the stream.
// Returns a vector<u1> filled with the bytes.
inline std::vector<u1> readBytes(std::istream &input, std::size_t count) {
    std::vector<u1> bytes(count);
    if (count == 0) {
        return bytes;
    }

    input.read(reinterpret_cast<char *>(bytes.data()),
               static_cast<std::streamsize>(count));
    if (input.gcount() != static_cast<std::streamsize>(count)) {
        throw std::runtime_error("unexpected end of stream while reading bytes");
    }

    return bytes;
}

// Convert a sequence of raw bytes to a std::string (useful for UTF-8 data
// stored in the constant pool).
inline std::string bytesToString(const std::vector<u1> &bytes) {
    return std::string(bytes.begin(), bytes.end());
}