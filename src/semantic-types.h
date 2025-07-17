#pragma once

#include <string>
#include <iostream>

// Basic data types supported by C--
enum class DataType {
    INT,         // integer type
    VOID,        // void type
    INT_ARRAY,   // integer array type
    UNKNOWN      // unknown/error type
};

// Symbol types for the symbol table
enum class SymbolType {
    VARIABLE,    // variable symbol
    FUNCTION,    // function symbol
    PARAMETER,   // parameter symbol
    BUILTIN      // built-in function symbol
};

// Error types for semantic analysis
enum class ErrorType {
    UNDEFINED_VARIABLE,
    UNDEFINED_FUNCTION,
    REDEFINITION,
    TYPE_MISMATCH,
    ARRAY_INDEX_NOT_INT,
    VOID_VARIABLE,
    FUNCTION_SIGNATURE_MISMATCH,
    RETURN_TYPE_MISMATCH,
    MAIN_FUNCTION_MISSING,
    MAIN_FUNCTION_INVALID,
    INVALID_ARRAY_ACCESS,
    INVALID_OPERATION
};

// Source location for error reporting
struct SourceLocation {
    int line;
    int column;

    SourceLocation(int l = 0, int c = 0) : line(l), column(c) {}
};

// Helper functions for type conversion
inline std::string dataTypeToString(DataType type) {
    switch (type) {
        case DataType::INT: return "int";
        case DataType::VOID: return "void";
        case DataType::INT_ARRAY: return "int[]";
        case DataType::UNKNOWN: return "unknown";
        default: return "unknown";
    }
}

inline DataType stringToDataType(const std::string& typeStr) {
    if (typeStr == "int") return DataType::INT;
    if (typeStr == "void") return DataType::VOID;
    return DataType::UNKNOWN;
}

inline std::string symbolTypeToString(SymbolType type) {
    switch (type) {
        case SymbolType::VARIABLE: return "variable";
        case SymbolType::FUNCTION: return "function";
        case SymbolType::PARAMETER: return "parameter";
        case SymbolType::BUILTIN: return "builtin";
        default: return "unknown";
    }
}

inline std::string errorTypeToString(ErrorType type) {
    switch (type) {
        case ErrorType::UNDEFINED_VARIABLE: return "Undefined Variable";
        case ErrorType::UNDEFINED_FUNCTION: return "Undefined Function";
        case ErrorType::REDEFINITION: return "Redefinition";
        case ErrorType::TYPE_MISMATCH: return "Type Mismatch";
        case ErrorType::ARRAY_INDEX_NOT_INT: return "Array Index Not Int";
        case ErrorType::VOID_VARIABLE: return "Void Variable";
        case ErrorType::FUNCTION_SIGNATURE_MISMATCH: return "Function Signature Mismatch";
        case ErrorType::RETURN_TYPE_MISMATCH: return "Return Type Mismatch";
        case ErrorType::MAIN_FUNCTION_MISSING: return "Main Function Missing";
        case ErrorType::MAIN_FUNCTION_INVALID: return "Main Function Invalid";
        case ErrorType::INVALID_ARRAY_ACCESS: return "Invalid Array Access";
        case ErrorType::INVALID_OPERATION: return "Invalid Operation";
        default: return "Unknown Error";
    }
}