#pragma once

#include "symbol.h"
#include <unordered_map>
#include <vector>
#include <memory>
#include <iostream>

// Hierarchical symbol table for scope management
class SymbolTable {
private:
    std::unordered_map<std::string, std::unique_ptr<Symbol>> symbols;
    SymbolTable* parent;
    std::vector<std::unique_ptr<SymbolTable>> children;
    int scope_level;

public:
    // Constructor for root scope
    SymbolTable() : parent(nullptr), scope_level(0) {}

    // Constructor for child scope
    SymbolTable(SymbolTable* p, int level) : parent(p), scope_level(level) {}

    // Destructor
    ~SymbolTable() = default;

    // Enter a new scope (create child scope)
    SymbolTable* enter_scope() {
        auto child = std::make_unique<SymbolTable>(this, scope_level + 1);
        SymbolTable* child_ptr = child.get();
        children.push_back(std::move(child));
        return child_ptr;
    }

    // Exit current scope (return to parent)
    SymbolTable* exit_scope() {
        return parent;
    }

    // Declare a new symbol in current scope
    bool declare_symbol(std::unique_ptr<Symbol> symbol) {
        const std::string& name = symbol->getName();

        // Check if symbol already exists in current scope
        if (symbols.find(name) != symbols.end()) {
            return false; // Symbol already exists
        }

        // Add symbol to current scope
        symbols[name] = std::move(symbol);
        return true;
    }

    // Look up a symbol starting from current scope and going up
    Symbol* lookup_symbol(const std::string& name) {
        // First check current scope
        auto it = symbols.find(name);
        if (it != symbols.end()) {
            return it->second.get();
        }

        // If not found and has parent, search parent scope
        if (parent != nullptr) {
            return parent->lookup_symbol(name);
        }

        // Symbol not found
        return nullptr;
    }

    // Look up a symbol only in current scope
    Symbol* lookup_current_scope(const std::string& name) {
        auto it = symbols.find(name);
        if (it != symbols.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    // Get scope level
    int get_scope_level() const {
        return scope_level;
    }

    // Get parent scope
    SymbolTable* get_parent() const {
        return parent;
    }

    // Print symbol table contents
    void print_table(int indent = 0) const {
        std::string indentation(indent * 2, ' ');
        std::cout << indentation << "=== Scope Level " << scope_level << " ===" << std::endl;
        
        for (const auto& [name, symbol] : symbols) {
            std::cout << indentation;
            symbol->print();
            std::cout << std::endl;
        }
        
        // Print child scopes
        for (const auto& child : children) {
            child->print_table(indent + 1);
        }
    }

    // Get symbol count
    size_t symbol_count() const {
        return symbols.size();
    }

    // Check if scope is empty
    bool is_empty() const {
        return symbols.empty();
    }
};