#pragma once

#include <string>
#include <vector>
#include <memory>
#include <iostream>

// Three-address code operation types
enum class OpCode {
    // Arithmetic operations
    ADD, SUB, MUL, DIV, MOD,
    
    // Comparison operations
    EQ, NE, LT, LE, GT, GE,
    
    // Logical operations
    AND, OR, NOT,
    
    // Assignment operations
    ASSIGN, COPY,
    
    // Control flow
    GOTO, IF_FALSE, IF_TRUE,
    
    // Function operations
    PARAM, CALL, RETURN,
    
    // Array operations
    ARRAY_ACCESS, ARRAY_ASSIGN,
    
    // Labels and markers
    LABEL, FUNCTION_BEGIN, FUNCTION_END,
    
    // Special operations
    NOP, HALT
};

// Convert opcode to string for debugging
inline std::string opCodeToString(OpCode op) {
    switch (op) {
        case OpCode::ADD: return "ADD";
        case OpCode::SUB: return "SUB";
        case OpCode::MUL: return "MUL";
        case OpCode::DIV: return "DIV";
        case OpCode::MOD: return "MOD";
        case OpCode::EQ: return "EQ";
        case OpCode::NE: return "NE";
        case OpCode::LT: return "LT";
        case OpCode::LE: return "LE";
        case OpCode::GT: return "GT";
        case OpCode::GE: return "GE";
        case OpCode::AND: return "AND";
        case OpCode::OR: return "OR";
        case OpCode::NOT: return "NOT";
        case OpCode::ASSIGN: return "ASSIGN";
        case OpCode::COPY: return "COPY";
        case OpCode::GOTO: return "GOTO";
        case OpCode::IF_FALSE: return "IF_FALSE";
        case OpCode::IF_TRUE: return "IF_TRUE";
        case OpCode::PARAM: return "PARAM";
        case OpCode::CALL: return "CALL";
        case OpCode::RETURN: return "RETURN";
        case OpCode::ARRAY_ACCESS: return "ARRAY_ACCESS";
        case OpCode::ARRAY_ASSIGN: return "ARRAY_ASSIGN";
        case OpCode::LABEL: return "LABEL";
        case OpCode::FUNCTION_BEGIN: return "FUNCTION_BEGIN";
        case OpCode::FUNCTION_END: return "FUNCTION_END";
        case OpCode::NOP: return "NOP";
        case OpCode::HALT: return "HALT";
        default: return "UNKNOWN";
    }
}

// IR instruction representing three-address code
class IRInstruction {
public:
    OpCode op;
    std::string result;
    std::string arg1;
    std::string arg2;
    int line_number;
    
    IRInstruction(OpCode operation, const std::string& res = "", 
                  const std::string& a1 = "", const std::string& a2 = "", int line = 0)
        : op(operation), result(res), arg1(a1), arg2(a2), line_number(line) {}
    
    // Convert instruction to string representation
    std::string to_string() const {
        std::string str = opCodeToString(op);
        
        if (!result.empty()) {
            str += " " + result;
        }
        
        if (!arg1.empty()) {
            str += ", " + arg1;
        }
        
        if (!arg2.empty()) {
            str += ", " + arg2;
        }
        
        return str;
    }
    
    // Check if instruction is a branch
    bool is_branch() const {
        return op == OpCode::GOTO || op == OpCode::IF_FALSE || op == OpCode::IF_TRUE;
    }
    
    // Check if instruction is a label
    bool is_label() const {
        return op == OpCode::LABEL;
    }
    
    // Check if instruction is a function call
    bool is_function_call() const {
        return op == OpCode::CALL;
    }
    
    // Check if instruction modifies the result
    bool modifies_result() const {
        return !result.empty() && op != OpCode::LABEL && op != OpCode::GOTO;
    }
    
    // Get variables used by this instruction
    std::vector<std::string> get_used_variables() const {
        std::vector<std::string> used;
        if (!arg1.empty() && !is_constant(arg1)) {
            used.push_back(arg1);
        }
        if (!arg2.empty() && !is_constant(arg2)) {
            used.push_back(arg2);
        }
        return used;
    }
    
    // Get variable defined by this instruction
    std::string get_defined_variable() const {
        if (modifies_result()) {
            return result;
        }
        return "";
    }
    
private:
    // Helper to check if a string is a constant
    bool is_constant(const std::string& str) const {
        return !str.empty() && (std::isdigit(str[0]) || str[0] == '-');
    }
};

// Type for a list of IR instructions
using IRCode = std::vector<IRInstruction>;
