#pragma once
#include "lexer.h"
#include "ast.h"
#include <vector>
#include <memory>
#include <stdexcept>

class Parser {
    std::vector<Token> tokens;
    size_t current = 0;

public:
    Parser(const std::vector<Token>& toks) : tokens(toks) {}

    std::unique_ptr<ASTNode> parse_program();
    std::unique_ptr<ASTNode> parse_declaration();
    std::unique_ptr<ASTNode> parse_var_declaration(const std::string& type, const std::string& name);
    std::unique_ptr<ASTNode> parse_fun_declaration(const std::string& type, const std::string& name);
    std::unique_ptr<Parameter> parse_param();
    std::vector<std::unique_ptr<Parameter>> parse_params();
    std::unique_ptr<ASTNode> parse_compound_stmt();
    std::unique_ptr<ASTNode> parse_statement();
    std::unique_ptr<ASTNode> parse_expression_stmt();
    std::unique_ptr<ASTNode> parse_selection_stmt();
    std::unique_ptr<ASTNode> parse_iteration_stmt();
    std::unique_ptr<ASTNode> parse_return_stmt();
    std::unique_ptr<ASTNode> parse_expression();
    std::unique_ptr<ASTNode> parse_var();
    std::unique_ptr<ASTNode> parse_simple_expression();
    std::unique_ptr<ASTNode> parse_additive_expression();
    std::unique_ptr<ASTNode> parse_term();
    std::unique_ptr<ASTNode> parse_factor();
    std::unique_ptr<ASTNode> parse_call();
    std::vector<std::unique_ptr<ASTNode>> parse_args();

    void error_recovery(const std::string& msg);
    void synchronize();

private:
    const Token& peek() const { return tokens[current]; }
    const Token& previous() const { return tokens[current - 1]; }
    bool match(TokenType type);
    bool check(TokenType type) const;
    Token advance();
    bool isAtEnd() const;
};