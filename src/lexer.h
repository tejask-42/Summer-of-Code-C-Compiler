#pragma once

#include <string>
#include <vector>

enum class TokenType {
    // Keywords
    Int, Void, If, Else, While, Return, Input, Output,
    // Identifiers & Literals
    Identifier, Number,
    Not,
    // Operators
    Plus, Minus, Star, Slash,
    Equal, EqualEqual, NotEqual,
    Less, LessEqual, Greater, GreaterEqual,
    // Delimiters
    LBrace, RBrace,
    LParen, RParen,
    LBracket, RBracket,
    Semicolon, Comma,
    // End-of-File
    Eof
};

inline std::string TokenTypeToString(TokenType type, const std::string& value) {
    switch (type) {
        case TokenType::Int: case TokenType::Void: case TokenType::If:
        case TokenType::Else: case TokenType::While: case TokenType::Return:
        case TokenType::Input: case TokenType::Output:
            return "KEYWORD(" + value + ")";
        case TokenType::Identifier:
            return "IDENTIFIER(" + value + ")";
        case TokenType::Number:
            return "NUMBER(" + value + ")";
        case TokenType::Plus:         return "PLUS(" + value + ")";
        case TokenType::Minus:        return "MINUS(" + value + ")";
        case TokenType::Star:         return "STAR(" + value + ")";
        case TokenType::Slash:        return "SLASH(" + value + ")";
        case TokenType::Equal:        return "EQUAL(" + value + ")";
        case TokenType::EqualEqual:   return "EQUALEQUAL(" + value + ")";
        case TokenType::NotEqual:     return "NOTEQUAL(" + value + ")";
        case TokenType::Less:         return "LESS(" + value + ")";
        case TokenType::LessEqual:    return "LESSEQUAL(" + value + ")";
        case TokenType::Greater:      return "GREATER(" + value + ")";
        case TokenType::GreaterEqual: return "GREATEREQUAL(" + value + ")";
        case TokenType::LBrace:       return "LBRACE(" + value + ")";
        case TokenType::RBrace:       return "RBRACE(" + value + ")";
        case TokenType::LParen:       return "LPAREN(" + value + ")";
        case TokenType::RParen:       return "RPAREN(" + value + ")";
        case TokenType::LBracket:     return "LBRACKET(" + value + ")";
        case TokenType::RBracket:     return "RBRACKET(" + value + ")";
        case TokenType::Semicolon:    return "SEMICOLON(" + value + ")";
        case TokenType::Comma:        return "COMMA(" + value + ")";
        case TokenType::Eof:          return "EOF";
        default:                      return "UNKNOWN(" + value + ")";
    }
}

class Token {
public:
    Token(TokenType type, const std::string& value, int line, int column);

    TokenType    type()   const;
    const std::string& value()  const;
    int          line()   const;
    int          column() const;

private:
    TokenType    type_;
    std::string  value_;
    int          line_;
    int          column_;
};

// The Lexer: turns raw source into a sequence of Tokens
class Lexer {
public:
    explicit Lexer(const std::string& source);

    std::vector<Token> tokenize();

    Token nextToken();

private:
    // Helpers for reading and classifying input
    char peekChar() const;
    char advance();
    void skipWhitespace();
    void handleComments();
    Token readNumber();
    Token readIdentifier();

    // Source buffer and position-tracking
    std::string source_;
    size_t      pos_    = 0;
    int         line_   = 1;
    int         column_ = 1;
};