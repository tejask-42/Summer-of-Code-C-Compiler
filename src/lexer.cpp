#include "lexer.h"
#include <cctype>
#include <stdexcept>

Token::Token(TokenType type, const std::string& value, int line, int column)
    : type_(type), value_(value), line_(line), column_(column) {}

TokenType Token::type() const     { return type_; }
const std::string& Token::value() const { return value_; }
int       Token::line() const     { return line_; }
int       Token::column() const   { return column_; }

Lexer::Lexer(const std::string& source)
    : source_(source), pos_(0), line_(1), column_(1) {}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (true) {
        Token tok = nextToken();
        tokens.push_back(tok);
        if (tok.type() == TokenType::Eof) break;
    }
    return tokens;
}

Token Lexer::nextToken() {
    skipWhitespace();
    handleComments();

    if (pos_ >= source_.size()) {
        return Token(TokenType::Eof, "", line_, column_);
    }

    char c = peekChar();

    if (std::isdigit(c)) {
        return readNumber();
    }
    if (std::isalpha(c) || c == '_') {
        return readIdentifier();
    }

    if (c == '=') {
        if (pos_ + 1 < source_.size() && source_[pos_ + 1] == '=') {
            advance(); advance();
            return Token(TokenType::EqualEqual, "==", line_, column_ - 1);
        } else {
            advance();
            return Token(TokenType::Equal, "=", line_, column_);
        }
    }
    if (c == '!') {
        if (pos_ + 1 < source_.size() && source_[pos_ + 1] == '=') {
            advance(); advance();
            return Token(TokenType::NotEqual, "!=", line_, column_ - 1);
        } else {
            advance();
            return Token(TokenType::Not, "!", line_, column_);
    }
}
    if (c == '<') {
        if (pos_ + 1 < source_.size() && source_[pos_ + 1] == '=') {
            advance(); advance();
            return Token(TokenType::LessEqual, "<=", line_, column_ - 1);
        } else {
            advance();
            return Token(TokenType::Less, "<", line_, column_);
        }
    }
    if (c == '>') {
        if (pos_ + 1 < source_.size() && source_[pos_ + 1] == '=') {
            advance(); advance();
            return Token(TokenType::GreaterEqual, ">=", line_, column_ - 1);
        } else {
            advance();
            return Token(TokenType::Greater, ">", line_, column_);
        }
    }

    switch (c) {
        case '+': advance(); return Token(TokenType::Plus, "+", line_, column_);
        case '-': advance(); return Token(TokenType::Minus, "-", line_, column_);
        case '*': advance(); return Token(TokenType::Star, "*", line_, column_);
        case '/': advance(); return Token(TokenType::Slash, "/", line_, column_);
        case '{': advance(); return Token(TokenType::LBrace, "{", line_, column_);
        case '}': advance(); return Token(TokenType::RBrace, "}", line_, column_);
        case '(': advance(); return Token(TokenType::LParen, "(", line_, column_);
        case ')': advance(); return Token(TokenType::RParen, ")", line_, column_);
        case '[': advance(); return Token(TokenType::LBracket, "[", line_, column_);
        case ']': advance(); return Token(TokenType::RBracket, "]", line_, column_);
        case ';': advance(); return Token(TokenType::Semicolon, ";", line_, column_);
        case ',': advance(); return Token(TokenType::Comma, ",", line_, column_);
        default:
            throw std::runtime_error(std::string("Unexpected character: ") + c);
    }
}


char Lexer::peekChar() const {
    if (pos_ >= source_.size()) return '\0';
    return source_[pos_];
}

char Lexer::advance() {
    char c = peekChar();
    pos_++;
    if (c == '\n') {
        line_++;
        column_ = 1;
    } else {
        column_++;
    }
    return c;
}

void Lexer::skipWhitespace() {
    while (std::isspace(peekChar())) {
        advance();
    }
}

void Lexer::handleComments() {
    if (peekChar() == '/' && pos_ + 1 < source_.size() && source_[pos_ + 1] == '/') {
        advance(); advance();
        while (peekChar() != '\n' && peekChar() != '\0') advance();
        if (peekChar() == '\n') advance();
        skipWhitespace();
        handleComments();
    }
    else if (peekChar() == '/' && pos_ + 1 < source_.size() && source_[pos_ + 1] == '*') {
        advance(); advance();
        while (pos_ + 1 < source_.size()) {
            if (peekChar() == '*' && source_[pos_ + 1] == '/') {
                advance(); advance();
                break;
            }
            advance();
        }
        skipWhitespace();
        handleComments();
    }
}

Token Lexer::readNumber() {
    std::string value;
    int start_col = column_;
    while (std::isdigit(peekChar())) {
        value += advance();
    }
    return Token(TokenType::Number, value, line_, start_col);
}

Token Lexer::readIdentifier() {
    std::string value;
    int start_col = column_;
    while (std::isalnum(peekChar()) || peekChar() == '_') {
        value += advance();
    }
    if (value == "int")    return Token(TokenType::Int, value, line_, start_col);
    if (value == "void")   return Token(TokenType::Void, value, line_, start_col);
    if (value == "if")     return Token(TokenType::If, value, line_, start_col);
    if (value == "else")   return Token(TokenType::Else, value, line_, start_col);
    if (value == "while")  return Token(TokenType::While, value, line_, start_col);
    if (value == "return") return Token(TokenType::Return, value, line_, start_col);
    if (value == "input")  return Token(TokenType::Input, value, line_, start_col);
    if (value == "output") return Token(TokenType::Output, value, line_, start_col);
    return Token(TokenType::Identifier, value, line_, start_col);
}