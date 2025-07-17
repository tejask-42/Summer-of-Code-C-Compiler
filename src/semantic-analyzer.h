#pragma once

#include "ast.h"
#include "symbol-table.h"
#include "semantic-error.h"
#include "semantic-types.h"
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

// Main semantic analyzer class using visitor pattern
class SemanticAnalyzer : public Visitor {
private:
    std::unique_ptr<SymbolTable> global_scope;
    SymbolTable* current_scope;
    SemanticErrorCollector error_collector;
    FunctionSymbol* current_function;
    
    // Node type annotations
    std::unordered_map<ASTNode*, DataType> node_types;
    
    // Helper methods for type checking
    DataType get_expression_type(ASTNode* expr);
    
    // Helper methods for analysis
    void initialize_builtin_functions();
    void check_main_function();
    
    // Type checking methods
    void check_assignment(BinaryOp& assignment);
    void check_binary_operation(BinaryOp& binary_op);
    void check_unary_operation(UnaryOp& unary_op);
    void check_function_call(Call& call);
    
    // Symbol lookup helpers
    VariableSymbol* get_variable_symbol(const std::string& name);
    FunctionSymbol* get_function_symbol(const std::string& name);
    
public:
    SemanticAnalyzer();
    ~SemanticAnalyzer() = default;
    
    // Main analysis entry point
    bool analyze(Program& program);
    
    // Get analysis results
    const SemanticErrorCollector& get_error_collector() const { return error_collector; }
    bool has_errors() const { return error_collector.has_errors(); }
    size_t get_error_count() const { return error_collector.error_count(); }
    
    // Print symbol table for debugging
    void print_symbol_table() const;
    
    // Get analysis summary
    std::string get_analysis_summary() const;
    
    // Reset analysis state
    void reset_analysis();
    
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
    
    // Utility methods
    bool is_builtin_function(const std::string& name);
    std::vector<std::string> get_builtin_functions();
    void annotate_node_type(ASTNode* node, DataType type);
    DataType get_node_type(ASTNode* node);
};