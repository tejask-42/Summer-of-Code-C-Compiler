#include "parser.h"

std::unique_ptr<ASTNode> Parser::parse_program() {
    std::cerr << "Entering parse_program\n";
    auto program = std::make_unique<Program>();
    while (!isAtEnd()) {
        std::cerr << "Parsing declaration at token: " << peek().value() << "\n";
        try {
            program->declarations.push_back(parse_declaration());
        } catch (const std::exception& e) {
            std::cerr << "Caught exception: " << e.what() << "\n";
            error_recovery(e.what());
        }
    }
    std::cerr << "Exiting parse_program\n";
    return program;
}

std::unique_ptr<ASTNode> Parser::parse_declaration() {
    if (check(TokenType::Int) || check(TokenType::Void)) {
        std::string type = peek().value();
        advance();
        if (check(TokenType::Identifier)) {
            std::string name = peek().value();
            advance();
            if (check(TokenType::LParen)) {
                return parse_fun_declaration(type, name);
            } else {
                return parse_var_declaration(type, name);
            }
        } else {
            throw std::runtime_error("Expected identifier after type");
        }
    }
    throw std::runtime_error("Expected declaration");
}

std::unique_ptr<ASTNode> Parser::parse_var_declaration(const std::string& type, const std::string& name) {
    int arraySize = -1;
    if (match(TokenType::LBracket)) {
        if (!check(TokenType::Number))
            throw std::runtime_error("Expected array size");
        arraySize = std::stoi(peek().value());
        advance();
        if (!match(TokenType::RBracket))
            throw std::runtime_error("Expected ']'");
    }
    if (!match(TokenType::Semicolon))
        throw std::runtime_error("Expected ';' after variable declaration");
    return std::make_unique<VarDeclaration>(type, name, arraySize);
}

std::unique_ptr<ASTNode, std::default_delete<ASTNode>> Parser::parse_fun_declaration(const std::string& type, const std::string& name) {
    match(TokenType::LParen);
    auto params = parse_params();
    if (!match(TokenType::RParen))
        throw std::runtime_error("Expected ')' after parameters");
    auto body = parse_compound_stmt();
    auto func = std::make_unique<FunDeclaration>(type, name);
    func->params = std::move(params);
    func->body = std::move(body);
    return func;
}

std::vector<std::unique_ptr<Parameter>> Parser::parse_params() {
    std::vector<std::unique_ptr<Parameter>> params;
    if (check(TokenType::Void)) {
        advance();
        return params;
    }
    if (check(TokenType::RParen)) {
        return params;
    }
    do {
        params.push_back(parse_param());
    } while (match(TokenType::Comma));
    return params;
}

std::unique_ptr<Parameter> Parser::parse_param() {
    std::string type = peek().value();
    advance();
    if (!check(TokenType::Identifier))
        throw std::runtime_error("Expected parameter name");
    std::string name = peek().value();
    advance();
    bool isArray = false;
    if (match(TokenType::LBracket)) {
        if (!match(TokenType::RBracket))
            throw std::runtime_error("Expected ']'");
        isArray = true;
    }
    return std::make_unique<Parameter>(type, name, isArray);
}

std::unique_ptr<ASTNode> Parser::parse_compound_stmt() {
    if (!match(TokenType::LBrace))
        throw std::runtime_error("Expected '{' at start of compound statement");
    auto compound = std::make_unique<CompoundStmt>();
    while (check(TokenType::Int) || check(TokenType::Void)) {
        std::string type = peek().value();
        advance();
        if (!check(TokenType::Identifier))
            throw std::runtime_error("Expected identifier after type");
        std::string name = peek().value();
        advance();
        compound->locals.push_back(
            std::unique_ptr<VarDeclaration>(
                static_cast<VarDeclaration*>(parse_var_declaration(type, name).release())
            )
        );
    }
    while (!check(TokenType::RBrace) && !isAtEnd()) {
        compound->statements.push_back(parse_statement());
    }
    if (!match(TokenType::RBrace))
        throw std::runtime_error("Expected '}' at end of compound statement");
    return compound;
}

std::unique_ptr<ASTNode> Parser::parse_statement() {
    if (check(TokenType::If)) return parse_selection_stmt();
    if (check(TokenType::While)) return parse_iteration_stmt();
    if (check(TokenType::Return)) return parse_return_stmt();
    if (check(TokenType::LBrace)) return parse_compound_stmt();
    return parse_expression_stmt();
}

std::unique_ptr<ASTNode> Parser::parse_expression_stmt() {
    if (match(TokenType::Semicolon))
        return nullptr; 
    auto expr = parse_expression();
    if (!match(TokenType::Semicolon))
        throw std::runtime_error("Expected ';' after expression");
    return expr;
}

std::unique_ptr<ASTNode> Parser::parse_selection_stmt() {
    match(TokenType::If);
    if (!match(TokenType::LParen))
        throw std::runtime_error("Expected '(' after 'if'");
    auto cond = parse_expression();
    if (!match(TokenType::RParen))
        throw std::runtime_error("Expected ')' after condition");
    auto thenStmt = parse_statement();
    std::unique_ptr<ASTNode> elseStmt = nullptr;
    if (match(TokenType::Else)) {
        elseStmt = parse_statement();
    }
    auto ifNode = std::make_unique<IfStmt>();
    ifNode->cond = std::move(cond);
    ifNode->thenStmt = std::move(thenStmt);
    ifNode->elseStmt = std::move(elseStmt);
    return ifNode;
}

std::unique_ptr<ASTNode> Parser::parse_iteration_stmt() {
    match(TokenType::While);
    if (!match(TokenType::LParen))
        throw std::runtime_error("Expected '(' after 'while'");
    auto cond = parse_expression();
    if (!match(TokenType::RParen))
        throw std::runtime_error("Expected ')' after condition");
    auto body = parse_statement();
    auto whileNode = std::make_unique<WhileStmt>();
    whileNode->cond = std::move(cond);
    whileNode->body = std::move(body);
    return whileNode;
}

std::unique_ptr<ASTNode> Parser::parse_return_stmt() {
    match(TokenType::Return);
    if (check(TokenType::Semicolon)) {
        advance();
        auto ret = std::make_unique<ReturnStmt>();
        ret->expr = nullptr;
        return ret;
    }
    auto expr = parse_expression();
    if (!match(TokenType::Semicolon))
        throw std::runtime_error("Expected ';' after return value");
    auto ret = std::make_unique<ReturnStmt>();
    ret->expr = std::move(expr);
    return ret;
}

std::unique_ptr<ASTNode> Parser::parse_expression() {
    if (check(TokenType::Identifier)) {
        if (check(TokenType::Identifier)) {
            if ((current + 1 < tokens.size()) && (tokens[current + 1].type() == TokenType::Equal)) {
                auto left = parse_var();
                match(TokenType::Equal);
                auto right = parse_expression();
                return std::make_unique<BinaryOp>("=", std::move(left), std::move(right));
    }
}
    }
    return parse_simple_expression();
}

std::unique_ptr<ASTNode> Parser::parse_var() {
    if (!check(TokenType::Identifier))
        throw std::runtime_error("Expected variable name");
    std::string name = peek().value();
    advance();
    if (match(TokenType::LBracket)) {
        auto index = parse_expression();
        if (!match(TokenType::RBracket))
            throw std::runtime_error("Expected ']'");
        return std::make_unique<Variable>(name, std::move(index));
    }
    return std::make_unique<Variable>(name);
}

std::unique_ptr<ASTNode> Parser::parse_simple_expression() {
    auto left = parse_additive_expression();
    if (check(TokenType::Less) || check(TokenType::LessEqual) ||
        check(TokenType::Greater) || check(TokenType::GreaterEqual) ||
        check(TokenType::EqualEqual) || check(TokenType::NotEqual)) {
        std::string op = peek().value();
        advance();
        auto right = parse_additive_expression();
        return std::make_unique<BinaryOp>(op, std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<ASTNode> Parser::parse_additive_expression() {
    auto left = parse_term();
    while (check(TokenType::Plus) || check(TokenType::Minus)) {
        std::string op = peek().value();
        advance();
        auto right = parse_term();
        left = std::make_unique<BinaryOp>(op, std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<ASTNode> Parser::parse_term() {
    auto left = parse_factor();
    while (check(TokenType::Star) || check(TokenType::Slash)) {
        std::string op = peek().value();
        advance();
        auto right = parse_factor();
        left = std::make_unique<BinaryOp>(op, std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<ASTNode> Parser::parse_factor() {
    if (match(TokenType::LParen)) {
        auto expr = parse_expression();
        if (!match(TokenType::RParen))
            throw std::runtime_error("Expected ')'");
        return expr;
    }
    if (check(TokenType::Identifier)) {
        if ((current + 1 < tokens.size()) && (tokens[current + 1].type() == TokenType::LParen)) {
            return parse_call();
        } else {
            return parse_var();
        }
    }
    if (check(TokenType::Number)) {
        int value = std::stoi(peek().value());
        advance();
        return std::make_unique<Number>(value);
    }
    if (match(TokenType::Minus)) {
        auto operand = parse_factor();
        return std::make_unique<UnaryOp>("-", std::move(operand));
    }
    if (match(TokenType::Not)) {
        auto operand = parse_factor();
        return std::make_unique<UnaryOp>("!", std::move(operand));
    }
    throw std::runtime_error("Expected expression");
}

std::unique_ptr<ASTNode> Parser::parse_call() {
    std::string name = peek().value();
    advance();
    match(TokenType::LParen);
    auto args = parse_args();
    if (!match(TokenType::RParen))
        throw std::runtime_error("Expected ')' after arguments");
    auto call = std::make_unique<Call>(name);
    call->args = std::move(args);
    return call;
}

std::vector<std::unique_ptr<ASTNode>> Parser::parse_args() {
    std::vector<std::unique_ptr<ASTNode>> args;
    if (check(TokenType::RParen)) return args;
    do {
        args.push_back(parse_expression());
    } while (match(TokenType::Comma));
    return args;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type() == type;
}

Token Parser::advance() {
    if (!isAtEnd()) current++;
    return previous();
}

bool Parser::isAtEnd() const {
    return peek().type() == TokenType::Eof;
}

void Parser::error_recovery(const std::string& msg) {
    std::cerr << "Syntax error at line " << peek().line()
              << ", col " << peek().column() << ": " << msg << std::endl;
    if (!isAtEnd()) advance();
    synchronize();
}

void Parser::synchronize() {
    while (!isAtEnd()) {
        if (previous().type() == TokenType::Semicolon) return;
        switch (peek().type()) {
            case TokenType::If:
            case TokenType::While:
            case TokenType::Return:
            case TokenType::Int:
            case TokenType::Void:
                return;
            default:
                advance();
        }
    }
}