#pragma once

#include "semantic-types.h"
#include <string>
#include <vector>
#include <sstream>
#include <iostream>

// Semantic error class for detailed error reporting
class SemanticError {
public:
    std::string message;
    SourceLocation location;
    ErrorType error_type;

    SemanticError(const std::string& msg, const SourceLocation& loc, ErrorType type)
        : message(msg), location(loc), error_type(type) {}

    // Format error message with location information
    std::string format_error() const {
        std::ostringstream oss;
        oss << "Semantic Error at line " << location.line 
            << ", column " << location.column << ": " << message;
        return oss.str();
    }

    // Get error type as string
    std::string get_error_type_string() const {
        return errorTypeToString(error_type);
    }
};

// Semantic error collector for managing multiple errors
class SemanticErrorCollector {
private:
    std::vector<SemanticError> errors;

public:
    // Add an error to the collection
    void add_error(const std::string& message, const SourceLocation& location, ErrorType type) {
        errors.emplace_back(message, location, type);
    }

    // Get all errors
    const std::vector<SemanticError>& get_errors() const {
        return errors;
    }

    // Check if there are any errors
    bool has_errors() const {
        return !errors.empty();
    }

    // Get error count
    size_t error_count() const {
        return errors.size();
    }

    // Clear all errors
    void clear_errors() {
        errors.clear();
    }

    // Print all errors
    void print_errors() const {
        if (errors.empty()) {
            std::cout << "No semantic errors found." << std::endl;
            return;
        }

        std::cout << "=== Semantic Errors (" << errors.size() << " found) ===" << std::endl;
        for (const auto& error : errors) {
            std::cout << error.format_error() << std::endl;
        }
    }

    // Create specific error messages
    void undefined_variable(const std::string& name, const SourceLocation& loc) {
        add_error("Undefined variable '" + name + "'", loc, ErrorType::UNDEFINED_VARIABLE);
    }

    void undefined_function(const std::string& name, const SourceLocation& loc) {
        add_error("Undefined function '" + name + "'", loc, ErrorType::UNDEFINED_FUNCTION);
    }

    void redefinition(const std::string& name, const SourceLocation& loc) {
        add_error("Redefinition of '" + name + "'", loc, ErrorType::REDEFINITION);
    }

    void type_mismatch(DataType expected, DataType actual, const SourceLocation& loc) {
        add_error("Type mismatch: expected " + dataTypeToString(expected) + 
                 ", got " + dataTypeToString(actual), loc, ErrorType::TYPE_MISMATCH);
    }

    void array_index_not_int(const SourceLocation& loc) {
        add_error("Array index must be of type int", loc, ErrorType::ARRAY_INDEX_NOT_INT);
    }

    void void_variable(const std::string& name, const SourceLocation& loc) {
        add_error("Variable '" + name + "' cannot be of type void", loc, ErrorType::VOID_VARIABLE);
    }

    void function_signature_mismatch(const std::string& name, const SourceLocation& loc) {
        add_error("Function call '" + name + "' does not match any declaration", 
                 loc, ErrorType::FUNCTION_SIGNATURE_MISMATCH);
    }

    void return_type_mismatch(DataType expected, DataType actual, const SourceLocation& loc) {
        add_error("Return type mismatch: expected " + dataTypeToString(expected) + 
                 ", got " + dataTypeToString(actual), loc, ErrorType::RETURN_TYPE_MISMATCH);
    }

    void main_function_missing() {
        add_error("Program must have a main function", SourceLocation(), ErrorType::MAIN_FUNCTION_MISSING);
    }

    void main_function_invalid(const SourceLocation& loc) {
        add_error("Main function must have signature: int main(void)", loc, ErrorType::MAIN_FUNCTION_INVALID);
    }
};