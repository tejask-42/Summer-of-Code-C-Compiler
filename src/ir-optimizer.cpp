#include "ir-optimizer.h"
#include <iostream>
#include <algorithm>
#include <sstream>

IRCode IROptimizer::optimize(const IRCode& instructions) {
    IRCode optimized = instructions;
    
    // Apply optimization passes
    constant_folding(optimized);
    copy_propagation(optimized);
    algebraic_simplification(optimized);
    dead_code_elimination(optimized);
    
    return optimized;
}

void IROptimizer::constant_folding(IRCode& instructions) {
    constant_map.clear();
    
    for (auto& instr : instructions) {
        // Check if both operands are constants
        if (is_constant(instr.arg1) && is_constant(instr.arg2)) {
            std::string result = evaluate_constant_expression(instr.op, instr.arg1, instr.arg2);
            if (!result.empty()) {
                // Replace with constant assignment
                instr.op = OpCode::ASSIGN;
                instr.arg1 = result;
                instr.arg2 = "";
                
                // Remember this constant
                constant_map[instr.result] = result;
            }
        }
        
        // Replace variables with their constant values
        if (constant_map.find(instr.arg1) != constant_map.end()) {
            instr.arg1 = constant_map[instr.arg1];
        }
        if (constant_map.find(instr.arg2) != constant_map.end()) {
            instr.arg2 = constant_map[instr.arg2];
        }
        
        // Track new constants
        if (instr.op == OpCode::ASSIGN && is_constant(instr.arg1)) {
            constant_map[instr.result] = instr.arg1;
        }
    }
}

void IROptimizer::dead_code_elimination(IRCode& instructions) {
    mark_used_variables(instructions);
    
    // Remove instructions that define unused variables
    instructions.erase(
        std::remove_if(instructions.begin(), instructions.end(),
            [this](const IRInstruction& instr) {
                return is_dead_code(instr, used_variables);
            }),
        instructions.end()
    );
}

void IROptimizer::copy_propagation(IRCode& instructions) {
    copy_map.clear();
    
    for (auto& instr : instructions) {
        // Replace variables with their copies
        if (copy_map.find(instr.arg1) != copy_map.end()) {
            instr.arg1 = copy_map[instr.arg1];
        }
        if (copy_map.find(instr.arg2) != copy_map.end()) {
            instr.arg2 = copy_map[instr.arg2];
        }
        
        // Track copy operations
        if (instr.op == OpCode::ASSIGN && !is_constant(instr.arg1)) {
            copy_map[instr.result] = instr.arg1;
        } else if (instr.op == OpCode::COPY) {
            copy_map[instr.result] = instr.arg1;
        }
        
        // Invalidate copies when variable is redefined
        if (instr.modifies_result()) {
            copy_map.erase(instr.result);
        }
    }
}

void IROptimizer::algebraic_simplification(IRCode& instructions) {
    for (auto& instr : instructions) {
        simplify_expression(instr);
    }
}

void IROptimizer::print_optimization_stats(const IRCode& before, const IRCode& after) {
    std::cout << "=== Optimization Statistics ===" << std::endl;
    std::cout << "Instructions before: " << before.size() << std::endl;
    std::cout << "Instructions after: " << after.size() << std::endl;
    std::cout << "Reduction: " << (before.size() - after.size()) << " instructions" << std::endl;
    std::cout << "===============================" << std::endl;
}

// Optimization passes and helpers
bool IROptimizer::is_constant(const std::string& str) {
    if (str.empty()) return false;
    return std::isdigit(str[0]) || (str[0] == '-' && str.length() > 1 && std::isdigit(str[1]));
}

int IROptimizer::get_constant_value(const std::string& str) {
    return std::stoi(str);
}

std::string IROptimizer::evaluate_constant_expression(OpCode op, const std::string& arg1, const std::string& arg2) {
    if (!is_constant(arg1) || !is_constant(arg2)) return "";
    
    int val1 = get_constant_value(arg1);
    int val2 = get_constant_value(arg2);
    int result;
    
    switch (op) {
        case OpCode::ADD: result = val1 + val2; break;
        case OpCode::SUB: result = val1 - val2; break;
        case OpCode::MUL: result = val1 * val2; break;
        case OpCode::DIV: 
            if (val2 == 0) return ""; // Division by zero
            result = val1 / val2; 
            break;
        case OpCode::MOD: 
            if (val2 == 0) return ""; // Division by zero
            result = val1 % val2; 
            break;
        case OpCode::EQ: result = (val1 == val2) ? 1 : 0; break;
        case OpCode::NE: result = (val1 != val2) ? 1 : 0; break;
        case OpCode::LT: result = (val1 < val2) ? 1 : 0; break;
        case OpCode::LE: result = (val1 <= val2) ? 1 : 0; break;
        case OpCode::GT: result = (val1 > val2) ? 1 : 0; break;
        case OpCode::GE: result = (val1 >= val2) ? 1 : 0; break;
        case OpCode::AND: result = (val1 && val2) ? 1 : 0; break;
        case OpCode::OR: result = (val1 || val2) ? 1 : 0; break;
        default: return "";
    }
    
    return std::to_string(result);
}

bool IROptimizer::is_dead_code(const IRInstruction& instr, const std::unordered_set<std::string>& used_vars) {
    // Don't remove labels, branches, calls, or returns
    if (instr.is_label() || instr.is_branch() || instr.is_function_call() || 
        instr.op == OpCode::RETURN || instr.op == OpCode::FUNCTION_BEGIN || 
        instr.op == OpCode::FUNCTION_END) {
        return false;
    }
    
    // Remove if result is not used
    std::string defined_var = instr.get_defined_variable();
    if (!defined_var.empty() && used_vars.find(defined_var) == used_vars.end()) {
        return true;
    }
    
    return false;
}

void IROptimizer::mark_used_variables(const IRCode& instructions) {
    used_variables.clear();
    
    // Mark all variables that are used
    for (const auto& instr : instructions) {
        auto used_vars = instr.get_used_variables();
        for (const auto& var : used_vars) {
            used_variables.insert(var);
        }
    }
}

bool IROptimizer::simplify_expression(IRInstruction& instr) {
    // x + 0 = x
    if (instr.op == OpCode::ADD && instr.arg2 == "0") {
        instr.op = OpCode::COPY;
        instr.arg2 = "";
        return true;
    }
    
    // x - 0 = x
    if (instr.op == OpCode::SUB && instr.arg2 == "0") {
        instr.op = OpCode::COPY;
        instr.arg2 = "";
        return true;
    }
    
    // x * 1 = x
    if (instr.op == OpCode::MUL && instr.arg2 == "1") {
        instr.op = OpCode::COPY;
        instr.arg2 = "";
        return true;
    }
    
    // x * 0 = 0
    if (instr.op == OpCode::MUL && instr.arg2 == "0") {
        instr.op = OpCode::ASSIGN;
        instr.arg1 = "0";
        instr.arg2 = "";
        return true;
    }
    
    // x / 1 = x
    if (instr.op == OpCode::DIV && instr.arg2 == "1") {
        instr.op = OpCode::COPY;
        instr.arg2 = "";
        return true;
    }
    
    return false;
}

bool IROptimizer::is_variable_used_after(const IRCode& instructions, size_t pos, const std::string& var) {
    for (size_t i = pos + 1; i < instructions.size(); ++i) {
        auto used_vars = instructions[i].get_used_variables();
        if (std::find(used_vars.begin(), used_vars.end(), var) != used_vars.end()) {
            return true;
        }
    }
    return false;
}
