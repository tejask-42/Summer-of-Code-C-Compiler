#pragma once

#include "ir-types.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>

class IROptimizer {
private:
    // Helper data structures for optimization
    std::unordered_map<std::string, std::string> copy_map;
    std::unordered_set<std::string> used_variables;
    std::unordered_map<std::string, std::string> constant_map;
    
    // Helper functions
    bool is_constant(const std::string& str);
    int get_constant_value(const std::string& str);
    std::string evaluate_constant_expression(OpCode op, const std::string& arg1, const std::string& arg2);
    bool is_dead_code(const IRInstruction& instr, const std::unordered_set<std::string>& used_vars);
    void mark_used_variables(const IRCode& instructions);
    
public:
    IROptimizer() = default;
    
    // Main optimization entry point
    IRCode optimize(const IRCode& instructions);
    
    // Individual optimization passes
    void constant_folding(IRCode& instructions);
    void dead_code_elimination(IRCode& instructions);
    void copy_propagation(IRCode& instructions);
    void algebraic_simplification(IRCode& instructions);
    
    // Utility functions
    void print_optimization_stats(const IRCode& before, const IRCode& after);
    
private:
    // Helper to remove redundant instructions
    void remove_redundant_instructions(IRCode& instructions);
    
    // Helper to simplify expressions
    bool simplify_expression(IRInstruction& instr);
    
    // Helper to check if variable is used after instruction
    bool is_variable_used_after(const IRCode& instructions, size_t pos, const std::string& var);
};
