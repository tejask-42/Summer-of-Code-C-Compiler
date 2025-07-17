#pragma once

#include "semantic-types.h"
#include <string>
#include <vector>
#include <memory>
#include <iostream>

// Base symbol class for all symbols in the symbol table
class Symbol {
public:
    std::string name;
    SymbolType symbol_type;
    int scope_level;
    SourceLocation location;

    Symbol(const std::string& n, SymbolType type, int level, const SourceLocation& loc)
        : name(n), symbol_type(type), scope_level(level), location(loc) {}

    virtual ~Symbol() = default;

    // Pure virtual function for type checking
    virtual DataType getDataType() const = 0;

    // Virtual function for printing symbol information
    virtual void print() const {
        std::cout << "Symbol: " << name << " (Type: " << symbolTypeToString(symbol_type) 
                  << ", Scope: " << scope_level << ")";
    }

    // Get symbol name
    const std::string& getName() const { return name; }

    // Get symbol type
    SymbolType getSymbolType() const { return symbol_type; }

    // Get scope level
    int getScopeLevel() const { return scope_level; }

    // Get source location
    const SourceLocation& getLocation() const { return location; }
};

// Variable symbol class
class VariableSymbol : public Symbol {
public:
    DataType data_type;
    bool is_array;
    int array_size;
    bool is_parameter;

    VariableSymbol(const std::string& n, DataType dt, bool array, int size, bool param, 
                   int level, const SourceLocation& loc)
        : Symbol(n, param ? SymbolType::PARAMETER : SymbolType::VARIABLE, level, loc),
          data_type(dt), is_array(array), array_size(size), is_parameter(param) {}

    DataType getDataType() const override {
        return is_array ? DataType::INT_ARRAY : data_type;
    }

    void print() const override {
        std::cout << "Variable: " << name << " (Type: " << dataTypeToString(data_type);
        if (is_array) {
            std::cout << "[" << (array_size > 0 ? std::to_string(array_size) : "") << "]";
        }
        std::cout << ", Scope: " << scope_level << ")";
    }
};

// Function symbol class
class FunctionSymbol : public Symbol {
public:
    DataType return_type;
    std::vector<std::unique_ptr<VariableSymbol>> parameters;
    bool is_defined;

    FunctionSymbol(const std::string& n, DataType rt, int level, const SourceLocation& loc)
        : Symbol(n, SymbolType::FUNCTION, level, loc), return_type(rt), is_defined(false) {}

    DataType getDataType() const override {
        return return_type;
    }

    void print() const override {
        std::cout << "Function: " << name << " (Return: " << dataTypeToString(return_type);
        std::cout << ", Parameters: ";
        for (const auto& param : parameters) {
            std::cout << dataTypeToString(param->data_type) << " ";
        }
        std::cout << ", Scope: " << scope_level << ")";
    }
};

// Built-in function symbol class
class BuiltinFunctionSymbol : public FunctionSymbol {
public:
    BuiltinFunctionSymbol(const std::string& n, DataType rt)
        : FunctionSymbol(n, rt, 0, SourceLocation()) {
        symbol_type = SymbolType::BUILTIN;
        is_defined = true;
    }

    void print() const override {
        std::cout << "Builtin Function: " << name << " (Return: " << dataTypeToString(return_type);
        std::cout << ", Parameters: ";
        for (const auto& param : parameters) {
            std::cout << dataTypeToString(param->data_type) << " ";
        }
        std::cout << ")";
    }
};