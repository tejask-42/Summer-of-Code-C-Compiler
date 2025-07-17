#pragma once

#include "ast.h"
#include "ir-types.h"
#include "semantic-analyzer.h"
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <sstream>

class IRGenerator : public Visitor {
private:
    IRCode instructions;
    int temp_counter;
    int label_counter;
    
    // Stack for function call parameters
    std::vector<std::string> param_stack;
    
    // Current function being processed
    std::string current_function;
    
    // Expression result storage
    std::string last_expression_result;
    
    // Symbol table reference for type information
    SemanticAnalyzer* analyzer;
    
public:
    IRGenerator(SemanticAnalyzer* semantic_analyzer = nullptr);
    
    // Main entry point for IR generation
    IRCode generate(Program& program);
    
    // Utility functions
    std::string new_temp();
    std::string new_label();
    void emit(OpCode op, const std::string& result = "", 
              const std::string& arg1 = "", const std::string& arg2 = "");
    
    // Get generated IR
    const IRCode& get_instructions() const { return instructions; }
    
    // Print IR for debugging
    void print_ir() const;
    
    // Clear IR for reuse
    void clear();
    
    // Visitor pattern implementation
    void visit(Program& node) override;
    void visit(VarDeclaration& node) override;
    void visit(FunDeclaration& node) override;
    void visit(Parameter& node) override;
    void visit(CompoundStmt& node) override;
    void visit(IfStmt& node) override;
    void visit(WhileStmt& node) override;
    void visit(ReturnStmt& node) override;
    void visit(BinaryOp& node) override;
    void visit(UnaryOp& node) override;
    void visit(Variable& node) override;
    void visit(Call& node) override;
    void visit(Number& node) override;
    void visit(ExpressionStmt& node) override;
    void visit(EmptyStmt& node) override;
    
private:
    // Helper functions for code generation
    void generate_expression(ASTNode* expr);
    void generate_assignment(BinaryOp& assignment);
    void generate_binary_operation(BinaryOp& binary_op);
    void generate_function_call(Call& call);
    void generate_array_access(Variable& var);
    void generate_comparison(BinaryOp& comparison);
    
    // Helper to get variable type
    std::string get_variable_type(const std::string& name);
    
    // Helper to check if variable is array
    bool is_array_variable(const std::string& name);
};
