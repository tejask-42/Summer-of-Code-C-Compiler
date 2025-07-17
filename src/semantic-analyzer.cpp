#include "semantic-analyzer.h"
#include <iostream>
#include <cassert>

SemanticAnalyzer::SemanticAnalyzer() 
    : global_scope(std::make_unique<SymbolTable>()), 
      current_scope(global_scope.get()), 
      current_function(nullptr) {
    initialize_builtin_functions();
}

void SemanticAnalyzer::initialize_builtin_functions() {
    // Add built-in input() function: int input(void)
    auto input_func = std::make_unique<BuiltinFunctionSymbol>("input", DataType::INT);
    current_scope->declare_symbol(std::move(input_func));
    
    // Add built-in output() function: void output(int)
    auto output_func = std::make_unique<BuiltinFunctionSymbol>("output", DataType::VOID);
    auto param = std::make_unique<VariableSymbol>("value", DataType::INT, false, -1, true, 0, SourceLocation());
    output_func->parameters.push_back(std::move(param));
    current_scope->declare_symbol(std::move(output_func));
}

bool SemanticAnalyzer::analyze(Program& program) {
    error_collector.clear_errors();
    
    // First pass: collect all function declarations
    for (const auto& decl : program.declarations) {
        if (auto func_decl = dynamic_cast<FunDeclaration*>(decl.get())) {
            DataType return_type = stringToDataType(func_decl->return_type);
            auto func_symbol = std::make_unique<FunctionSymbol>(
                func_decl->name, return_type, 0, SourceLocation());
            
            // Add parameters
            // for (const auto& param : func_decl->params) {
            //     DataType param_type = stringToDataType(param->type);
            //     auto param_symbol = std::make_unique<VariableSymbol>(
            //         param->name, param_type, param->isArray, -1, true, 0, SourceLocation());
            //     func_symbol->parameters.push_back(std::move(param_symbol));
            // }
            
            func_symbol->is_defined = (func_decl->body != nullptr);
            
            if (!current_scope->declare_symbol(std::move(func_symbol))) {
                error_collector.redefinition(func_decl->name, SourceLocation());
            }
        }
    }
    
    // Second pass: analyze all declarations
    for (const auto& decl : program.declarations) {
        decl->accept(*this);
    }
    
    // Check for main function
    check_main_function();
    
    return !error_collector.has_errors();
}

void SemanticAnalyzer::visit(Program& node) {
    // Program analysis is handled in analyze() method
    (void)node; // Prevent unused parameter warning
}

void SemanticAnalyzer::visit(VarDeclaration& node) {
    DataType var_type = stringToDataType(node.type);
    if (var_type == DataType::VOID) {
        error_collector.void_variable(node.name, SourceLocation());
        return;
    }
    
    bool is_array = (node.arraySize != -1);
    auto var_symbol = std::make_unique<VariableSymbol>(
        node.name, var_type, is_array, node.arraySize, false, 
        current_scope->get_scope_level(), SourceLocation());
    
    if (!current_scope->declare_symbol(std::move(var_symbol))) {
        error_collector.redefinition(node.name, SourceLocation());
    }
}

void SemanticAnalyzer::visit(FunDeclaration& node) {
    // Lookup already-declared function
    auto func_symbol = dynamic_cast<FunctionSymbol*>(current_scope->lookup_symbol(node.name));
    if (!func_symbol) {
        error_collector.undefined_function(node.name, SourceLocation());
        return;
    }

    current_function = func_symbol;
    current_scope = current_scope->enter_scope();

    // Analyze and store parameters
    for (const auto& param : node.params) {
        DataType param_type = stringToDataType(param->type);
        bool is_array = param->isArray;

        auto param_symbol = std::make_unique<VariableSymbol>(
            param->name,
            param_type,
            is_array,
            -1,
            true,
            current_scope->get_scope_level(),
            SourceLocation()
        );

        // Add to both function symbol and scope
        func_symbol->parameters.push_back(std::make_unique<VariableSymbol>(*param_symbol));
        if (!current_scope->declare_symbol(std::move(param_symbol))) {
            error_collector.redefinition(param->name, SourceLocation());
        }
    }

    // Visit function body
    if (node.body) {
        node.body->accept(*this);
    }

    current_scope = current_scope->exit_scope();
    current_function = nullptr;
}

void SemanticAnalyzer::visit(Parameter& node) {
    DataType param_type = stringToDataType(node.type);
    
    if (param_type == DataType::VOID && !node.isArray) {
        error_collector.void_variable(node.name, SourceLocation());
        return;
    }
    
    auto param_symbol = std::make_unique<VariableSymbol>(
        node.name, param_type, node.isArray, -1, true, 
        current_scope->get_scope_level(), SourceLocation());
    
    if (!current_scope->declare_symbol(std::move(param_symbol))) {
        error_collector.redefinition(node.name, SourceLocation());
    }
}

void SemanticAnalyzer::visit(CompoundStmt& node) {
    current_scope = current_scope->enter_scope();
    
    // Analyze local declarations
    for (const auto& local : node.locals) {
        local->accept(*this);
    }
    
    // Analyze statements
    for (const auto& stmt : node.statements) {
        if (stmt) {
            stmt->accept(*this);
        }
    }
    
    current_scope = current_scope->exit_scope();
}

void SemanticAnalyzer::visit(IfStmt& node) {
    if (node.cond) {
        node.cond->accept(*this);
        DataType cond_type = get_expression_type(node.cond.get());
        if (cond_type != DataType::INT) {
            error_collector.type_mismatch(DataType::INT, cond_type, SourceLocation());
        }
    }
    
    if (node.thenStmt) node.thenStmt->accept(*this);
    if (node.elseStmt) node.elseStmt->accept(*this);
}

void SemanticAnalyzer::visit(WhileStmt& node) {
    if (node.cond) {
        node.cond->accept(*this);
        DataType cond_type = get_expression_type(node.cond.get());
        if (cond_type != DataType::INT) {
            error_collector.type_mismatch(DataType::INT, cond_type, SourceLocation());
        }
    }
    
    if (node.body) {
        node.body->accept(*this);
    }
}

void SemanticAnalyzer::visit(ReturnStmt& node) {
    if (!current_function) {
        error_collector.add_error("Return statement outside function", SourceLocation(), ErrorType::RETURN_TYPE_MISMATCH);
        return;
    }
    
    if (node.expr) {
        node.expr->accept(*this);
        DataType expr_type = get_expression_type(node.expr.get());
        if (expr_type != current_function->return_type) {
            error_collector.return_type_mismatch(current_function->return_type, expr_type, SourceLocation());
        }
    } else {
        if (current_function->return_type != DataType::VOID) {
            error_collector.return_type_mismatch(current_function->return_type, DataType::VOID, SourceLocation());
        }
    }
}

void SemanticAnalyzer::visit(BinaryOp& node) {
    if (node.left) node.left->accept(*this);
    if (node.right) node.right->accept(*this);
    
    if (node.op == "=") {
        check_assignment(node);
    } else {
        check_binary_operation(node);
    }
}

void SemanticAnalyzer::visit(UnaryOp& node) {
    if (node.operand) {
        node.operand->accept(*this);
        check_unary_operation(node);
    }
}

void SemanticAnalyzer::visit(Variable& node) {
    auto var_symbol = get_variable_symbol(node.name);
    if (!var_symbol) {
        error_collector.undefined_variable(node.name, SourceLocation());
        return;
    }
    
    if (node.index) {
        node.index->accept(*this);
        DataType index_type = get_expression_type(node.index.get());
        if (index_type != DataType::INT) {
            error_collector.array_index_not_int(SourceLocation());
        }
        
        if (!var_symbol->is_array) {
            error_collector.add_error("Index applied to non-array variable '" + node.name + "'", 
                                    SourceLocation(), ErrorType::TYPE_MISMATCH);
        }
    }
}

void SemanticAnalyzer::visit(Call& node) {
    check_function_call(node);
}

void SemanticAnalyzer::visit(Number& node) {
    // Numbers are always int type - no checks needed
    (void)node; // Prevent unused parameter warning
}

void SemanticAnalyzer::visit(ExpressionStmt& node) {
    if (node.expr) {
        node.expr->accept(*this);
    }
}

void SemanticAnalyzer::visit(EmptyStmt& node) {
    // Empty statement - nothing to check
    (void)node; // Prevent unused parameter warning
}

DataType SemanticAnalyzer::get_expression_type(ASTNode* expr) {
    if (auto num = dynamic_cast<Number*>(expr)) {
        return DataType::INT;
    }
    
    if (auto var = dynamic_cast<Variable*>(expr)) {
        auto var_symbol = get_variable_symbol(var->name);
        if (!var_symbol) return DataType::UNKNOWN;
        
        if (var->index) {
            return DataType::INT; // Array access returns int
        } else {
            return var_symbol->is_array ? DataType::INT_ARRAY : var_symbol->data_type;
        }
    }
    
    if (auto call = dynamic_cast<Call*>(expr)) {
        auto func_symbol = get_function_symbol(call->name);
        if (!func_symbol) return DataType::UNKNOWN;
        return func_symbol->return_type;
    }
    
    if (auto binary = dynamic_cast<BinaryOp*>(expr)) {
        return DataType::INT; // All binary operations return int
    }
    
    if (auto unary = dynamic_cast<UnaryOp*>(expr)) {
        return DataType::INT; // All unary operations return int
    }
    
    return DataType::UNKNOWN;
}

void SemanticAnalyzer::check_assignment(BinaryOp& assignment) {
    auto left_var = dynamic_cast<Variable*>(assignment.left.get());
    if (!left_var) {
        error_collector.add_error("Left side of assignment must be a variable", 
                                SourceLocation(), ErrorType::TYPE_MISMATCH);
        return;
    }
    
    DataType left_type = get_expression_type(assignment.left.get());
    DataType right_type = get_expression_type(assignment.right.get());
    
    if (left_type == DataType::INT_ARRAY && right_type == DataType::INT_ARRAY) {
        error_collector.add_error("Cannot assign arrays", SourceLocation(), ErrorType::TYPE_MISMATCH);
    } else if (left_type != right_type && left_type != DataType::INT) {
        error_collector.type_mismatch(left_type, right_type, SourceLocation());
    }
}

void SemanticAnalyzer::check_binary_operation(BinaryOp& binary_op) {
    DataType left_type = get_expression_type(binary_op.left.get());
    DataType right_type = get_expression_type(binary_op.right.get());
    
    if (left_type != DataType::INT || right_type != DataType::INT) {
        error_collector.add_error("Binary operation requires integer operands", 
                                SourceLocation(), ErrorType::TYPE_MISMATCH);
    }
}

void SemanticAnalyzer::check_unary_operation(UnaryOp& unary_op) {
    DataType operand_type = get_expression_type(unary_op.operand.get());
    
    if (operand_type != DataType::INT) {
        error_collector.add_error("Unary operation requires integer operand", 
                                SourceLocation(), ErrorType::TYPE_MISMATCH);
    }
}

void SemanticAnalyzer::check_function_call(Call& call) {
    auto func_symbol = get_function_symbol(call.name);
    if (!func_symbol) {
        error_collector.undefined_function(call.name, SourceLocation());
        return;
    }

    if (call.args.size() != func_symbol->parameters.size()) {
        error_collector.function_signature_mismatch(call.name, SourceLocation());
        return;
    }

    for (size_t i = 0; i < call.args.size(); ++i) {
        call.args[i]->accept(*this);

        DataType arg_type = get_expression_type(call.args[i].get());
        DataType param_type = func_symbol->parameters[i]->getDataType();

        // Acceptable: INT passed to INT_PARAM, INT[] to INT[]
        // Reject: ARRAY to INT, or INT to ARRAY with indexing
        if ((arg_type == DataType::INT && param_type == DataType::INT) ||
            (arg_type == DataType::INT_ARRAY && param_type == DataType::INT_ARRAY)) {
            continue;
        } else {
            error_collector.type_mismatch(param_type, arg_type, SourceLocation());
        }
    }
}

void SemanticAnalyzer::check_main_function() {
    auto main_func = get_function_symbol("main");
    if (!main_func) {
        error_collector.main_function_missing();
        return;
    }
    
    // Check main function signature: int main(void)
    if (main_func->return_type != DataType::INT || !main_func->parameters.empty()) {
        error_collector.main_function_invalid(SourceLocation());
    }
}

VariableSymbol* SemanticAnalyzer::get_variable_symbol(const std::string& name) {
    Symbol* symbol = current_scope->lookup_symbol(name);
    if (symbol && symbol->symbol_type == SymbolType::VARIABLE) {
        return dynamic_cast<VariableSymbol*>(symbol);
    }
    return nullptr;
}

FunctionSymbol* SemanticAnalyzer::get_function_symbol(const std::string& name) {
    Symbol* symbol = current_scope->lookup_symbol(name);
    if (symbol && (symbol->symbol_type == SymbolType::FUNCTION || symbol->symbol_type == SymbolType::BUILTIN)) {
        return dynamic_cast<FunctionSymbol*>(symbol);
    }
    return nullptr;
}

void SemanticAnalyzer::print_symbol_table() const {
    std::cout << "=== Symbol Table ===" << std::endl;
    global_scope->print_table();
    std::cout << "===================" << std::endl;
}

std::string SemanticAnalyzer::get_analysis_summary() const {
    std::ostringstream oss;
    oss << "Semantic Analysis Summary:" << std::endl;
    oss << "  Error Count: " << error_collector.error_count() << std::endl;
    oss << "  Status: " << (error_collector.has_errors() ? "FAILED" : "PASSED") << std::endl;
    return oss.str();
}

void SemanticAnalyzer::reset_analysis() {
    error_collector.clear_errors();
    current_function = nullptr;
    current_scope = global_scope.get();
    node_types.clear();
}

bool SemanticAnalyzer::is_builtin_function(const std::string& name) {
    return (name == "input" || name == "output");
}

std::vector<std::string> SemanticAnalyzer::get_builtin_functions() {
    return {"input", "output"};
}

void SemanticAnalyzer::annotate_node_type(ASTNode* node, DataType type) {
    if (node) {
        node_types[node] = type;
    }
}

DataType SemanticAnalyzer::get_node_type(ASTNode* node) {
    auto it = node_types.find(node);
    if (it != node_types.end()) {
        return it->second;
    }
    return DataType::UNKNOWN;
}
