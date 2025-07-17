#include "ir-generator.h"
#include <iostream>
#include <sstream>

IRGenerator::IRGenerator(SemanticAnalyzer* semantic_analyzer)
    : temp_counter(0), label_counter(0), analyzer(semantic_analyzer) {}

IRCode IRGenerator::generate(Program& program) {
    clear();
    visit(program);
    return instructions;
}

std::string IRGenerator::new_temp() {
    return "t" + std::to_string(temp_counter++);
}

std::string IRGenerator::new_label() {
    return "L" + std::to_string(label_counter++);
}

void IRGenerator::emit(OpCode op, const std::string& result, 
                      const std::string& arg1, const std::string& arg2) {
    instructions.emplace_back(op, result, arg1, arg2);
}

void IRGenerator::print_ir() const {
    std::cout << "=== Generated IR ===" << std::endl;
    for (size_t i = 0; i < instructions.size(); ++i) {
        std::cout << i << ": " << instructions[i].to_string() << std::endl;
    }
    std::cout << "===================" << std::endl;
}

void IRGenerator::clear() {
    instructions.clear();
    temp_counter = 0;
    label_counter = 0;
    param_stack.clear();
    current_function.clear();
    last_expression_result.clear();
}

void IRGenerator::visit(Program& node) {
    // Generate IR for all declarations
    for (const auto& decl : node.declarations) {
        decl->accept(*this);
    }
}

void IRGenerator::visit(VarDeclaration& node) {
    // Global variable declarations don't generate IR instructions
    // They are handled by the runtime/symbol table
    (void)node;
}

void IRGenerator::visit(FunDeclaration& node) {
    current_function = node.name;
    
    // Function begin marker
    emit(OpCode::FUNCTION_BEGIN, node.name);
    
    // Function body
    if (node.body) {
        node.body->accept(*this);
    }
    
    // Function end marker
    emit(OpCode::FUNCTION_END, node.name);
    
    current_function.clear();
}

void IRGenerator::visit(Parameter& node) {
    // Parameters are handled during function calls
    // No IR generation needed here
    (void)node;
}

void IRGenerator::visit(CompoundStmt& node) {
    // Process local declarations
    for (const auto& local : node.locals) {
        local->accept(*this);
    }
    
    // Process statements
    for (const auto& stmt : node.statements) {
        if (stmt) {
            stmt->accept(*this);
        }
    }
}

void IRGenerator::visit(IfStmt& node) {
    std::string else_label = new_label();
    std::string end_label = new_label();
    
    // Generate condition
    if (node.cond) {
        generate_expression(node.cond.get());
        emit(OpCode::IF_FALSE, else_label, last_expression_result);
    }
    
    // Generate then statement
    if (node.thenStmt) {
        node.thenStmt->accept(*this);
    }
    
    // Jump to end if no else
    if (node.elseStmt) {
        emit(OpCode::GOTO, end_label);
    }
    
    // Else label
    emit(OpCode::LABEL, else_label);
    
    // Generate else statement
    if (node.elseStmt) {
        node.elseStmt->accept(*this);
    }
    
    // End label
    if (node.elseStmt) {
        emit(OpCode::LABEL, end_label);
    }
}

void IRGenerator::visit(WhileStmt& node) {
    std::string loop_label = new_label();
    std::string end_label = new_label();
    
    // Loop start
    emit(OpCode::LABEL, loop_label);
    
    // Generate condition
    if (node.cond) {
        generate_expression(node.cond.get());
        emit(OpCode::IF_FALSE, end_label, last_expression_result);
    }
    
    // Generate body
    if (node.body) {
        node.body->accept(*this);
    }
    
    // Jump back to loop start
    emit(OpCode::GOTO, loop_label);
    
    // End label
    emit(OpCode::LABEL, end_label);
}

void IRGenerator::visit(ReturnStmt& node) {
    if (node.expr) {
        generate_expression(node.expr.get());
        emit(OpCode::RETURN, "", last_expression_result);
    } else {
        emit(OpCode::RETURN);
    }
}

void IRGenerator::visit(BinaryOp& node) {
    if (node.op == "=") {
        generate_assignment(node);
    } else {
        generate_binary_operation(node);
    }
}

void IRGenerator::visit(UnaryOp& node) {
    if (node.operand) {
        generate_expression(node.operand.get());
        
        std::string result = new_temp();
        
        if (node.op == "-") {
            emit(OpCode::SUB, result, "0", last_expression_result);
        } else if (node.op == "!") {
            emit(OpCode::NOT, result, last_expression_result);
        }
        
        last_expression_result = result;
    }
}

void IRGenerator::visit(Variable& node) {
    if (node.index) {
        generate_array_access(node);
    } else {
        last_expression_result = node.name;
    }
}

void IRGenerator::visit(Call& node) {
    generate_function_call(node);
}

void IRGenerator::visit(Number& node) {
    last_expression_result = std::to_string(node.value);
}

void IRGenerator::visit(ExpressionStmt& node) {
    if (node.expr) {
        generate_expression(node.expr.get());
    }
}

void IRGenerator::visit(EmptyStmt& node) {
    // Empty statement generates no IR
    (void)node;
}

void IRGenerator::generate_expression(ASTNode* expr) {
    if (expr) {
        expr->accept(*this);
    }
}

void IRGenerator::generate_assignment(BinaryOp& assignment) {
    // Generate right side
    generate_expression(assignment.right.get());
    std::string rhs_result = last_expression_result;
    
    // Handle left side
    if (auto var = dynamic_cast<Variable*>(assignment.left.get())) {
        if (var->index) {
            // Array assignment
            generate_expression(var->index.get());
            std::string index_result = last_expression_result;
            emit(OpCode::ARRAY_ASSIGN, var->name, index_result, rhs_result);
        } else {
            // Simple assignment
            emit(OpCode::ASSIGN, var->name, rhs_result);
        }
        last_expression_result = var->name;
    }
}

void IRGenerator::generate_binary_operation(BinaryOp& binary_op) {
    // Generate left operand
    generate_expression(binary_op.left.get());
    std::string left_result = last_expression_result;
    
    // Generate right operand
    generate_expression(binary_op.right.get());
    std::string right_result = last_expression_result;
    
    // Generate operation
    std::string result = new_temp();
    OpCode op_code;
    
    if (binary_op.op == "+") op_code = OpCode::ADD;
    else if (binary_op.op == "-") op_code = OpCode::SUB;
    else if (binary_op.op == "*") op_code = OpCode::MUL;
    else if (binary_op.op == "/") op_code = OpCode::DIV;
    else if (binary_op.op == "%") op_code = OpCode::MOD;
    else if (binary_op.op == "==") op_code = OpCode::EQ;
    else if (binary_op.op == "!=") op_code = OpCode::NE;
    else if (binary_op.op == "<") op_code = OpCode::LT;
    else if (binary_op.op == "<=") op_code = OpCode::LE;
    else if (binary_op.op == ">") op_code = OpCode::GT;
    else if (binary_op.op == ">=") op_code = OpCode::GE;
    else if (binary_op.op == "&&") op_code = OpCode::AND;
    else if (binary_op.op == "||") op_code = OpCode::OR;
    else {
        op_code = OpCode::NOP; // Unknown operation
    }
    
    emit(op_code, result, left_result, right_result);
    last_expression_result = result;
}

void IRGenerator::generate_function_call(Call& call) {
    // Generate arguments in reverse order (for stack-based parameter passing)
    for (int i = call.args.size() - 1; i >= 0; --i) {
        generate_expression(call.args[i].get());
        emit(OpCode::PARAM, "", last_expression_result);
    }
    
    // Generate call
    std::string result = new_temp();
    emit(OpCode::CALL, result, call.name, std::to_string(call.args.size()));
    last_expression_result = result;
}

void IRGenerator::generate_array_access(Variable& var) {
    // Generate index expression
    generate_expression(var.index.get());
    std::string index_result = last_expression_result;
    
    // Generate array access
    std::string result = new_temp();
    emit(OpCode::ARRAY_ACCESS, result, var.name, index_result);
    last_expression_result = result;
}

std::string IRGenerator::get_variable_type(const std::string& name) {
    // Use semantic analyzer to get type information
    return "int"; // Default for now
}

bool IRGenerator::is_array_variable(const std::string& name) {
    // Use semantic analyzer to check if variable is array
    return false; // Default for now
}
