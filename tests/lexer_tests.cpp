#include "lexer.h"
#include <gtest/gtest.h>

// Sample test: keywords and simple tokens
TEST(LexerTest, KeywordsAndOperators) {
    std::string src = "int x = 42;";
    Lexer lex(src);
    auto tokens = lex.tokenize();

    ASSERT_EQ(tokens.size(), 6);  // int, id, =, number, ;, eof
    EXPECT_EQ(tokens[0].type(), TokenType::Int);
    EXPECT_EQ(tokens[1].type(), TokenType::Identifier);
    EXPECT_EQ(tokens[2].type(), TokenType::Equal);
    EXPECT_EQ(tokens[3].type(), TokenType::Number);
    EXPECT_EQ(tokens[4].type(), TokenType::Semicolon);
    EXPECT_EQ(tokens[5].type(), TokenType::Eof);
}

TEST(LexerTest, AllKeywords) {
    std::string src = "int void if else while return input output";
    Lexer lex(src);
    auto tokens = lex.tokenize();
    ASSERT_EQ(tokens.size(), 9); // 8 keywords + eof
    EXPECT_EQ(tokens[0].type(), TokenType::Int);
    EXPECT_EQ(tokens[1].type(), TokenType::Void);
    EXPECT_EQ(tokens[2].type(), TokenType::If);
    EXPECT_EQ(tokens[3].type(), TokenType::Else);
    EXPECT_EQ(tokens[4].type(), TokenType::While);
    EXPECT_EQ(tokens[5].type(), TokenType::Return);
    EXPECT_EQ(tokens[6].type(), TokenType::Input);
    EXPECT_EQ(tokens[7].type(), TokenType::Output);
    EXPECT_EQ(tokens[8].type(), TokenType::Eof);
}

TEST(LexerTest, OperatorsAndDelimiters) {
    std::string src = "+ - * / = == != < <= > >= { } ( ) [ ] ; ,";
    Lexer lex(src);
    auto tokens = lex.tokenize();
    std::vector<TokenType> expected = {
        TokenType::Plus, TokenType::Minus, TokenType::Star, TokenType::Slash,
        TokenType::Equal, TokenType::EqualEqual, TokenType::NotEqual,
        TokenType::Less, TokenType::LessEqual, TokenType::Greater, TokenType::GreaterEqual,
        TokenType::LBrace, TokenType::RBrace, TokenType::LParen, TokenType::RParen,
        TokenType::LBracket, TokenType::RBracket, TokenType::Semicolon, TokenType::Comma,
        TokenType::Eof
    };
    ASSERT_EQ(tokens.size(), expected.size());
    for (size_t i = 0; i < expected.size(); ++i) {
        EXPECT_EQ(tokens[i].type(), expected[i]);
    }
}

TEST(LexerTest, IdentifiersAndNumbers) {
    std::string src = "foo bar123 _baz 007";
    Lexer lex(src);
    auto tokens = lex.tokenize();
    ASSERT_EQ(tokens.size(), 5); // 4 identifiers/numbers + eof
    EXPECT_EQ(tokens[0].type(), TokenType::Identifier);
    EXPECT_EQ(tokens[1].type(), TokenType::Identifier);
    EXPECT_EQ(tokens[2].type(), TokenType::Identifier);
    EXPECT_EQ(tokens[3].type(), TokenType::Number);
    EXPECT_EQ(tokens[4].type(), TokenType::Eof);
}

TEST(LexerTest, CommentsAreIgnored) {
    std::string src = "int x; // line comment\n/* block\ncomment */x = 1;";
    Lexer lex(src);
    auto tokens = lex.tokenize();
    std::vector<TokenType> expected = {
        TokenType::Int, TokenType::Identifier, TokenType::Semicolon,
        TokenType::Identifier, TokenType::Equal, TokenType::Number, TokenType::Semicolon,
        TokenType::Eof
    };
    ASSERT_EQ(tokens.size(), expected.size());
    for (size_t i = 0; i < expected.size(); ++i) {
        EXPECT_EQ(tokens[i].type(), expected[i]);
    }
}
TEST(LexerTest, EmptyInput) {
    std::string src = "";
    Lexer lex(src);
    auto tokens = lex.tokenize();
    ASSERT_EQ(tokens.size(), 1);
    EXPECT_EQ(tokens[0].type(), TokenType::Eof);
}

TEST(LexerTest, OnlyWhitespace) {
    std::string src = "   \n\t  ";
    Lexer lex(src);
    auto tokens = lex.tokenize();
    ASSERT_EQ(tokens.size(), 1);
    EXPECT_EQ(tokens[0].type(), TokenType::Eof);
}

TEST(LexerTest, OnlyComments) {
    std::string src = "// comment\n/* block comment */";
    Lexer lex(src);
    auto tokens = lex.tokenize();
    ASSERT_EQ(tokens.size(), 1);
    EXPECT_EQ(tokens[0].type(), TokenType::Eof);
}

TEST(LexerTest, InvalidTokenThrows) {
    std::string src = "@";
    Lexer lex(src);
    EXPECT_THROW(lex.tokenize(), std::runtime_error);
}

TEST(LexerTest, LeadingZerosAndLargeNumbers) {
    std::string src = "007 12345678901234567890";
    Lexer lex(src);
    auto tokens = lex.tokenize();
    ASSERT_EQ(tokens.size(), 3); // two numbers + eof
    EXPECT_EQ(tokens[0].type(), TokenType::Number);
    EXPECT_EQ(tokens[0].value(), "007");
    EXPECT_EQ(tokens[1].type(), TokenType::Number);
    EXPECT_EQ(tokens[1].value(), "12345678901234567890");
    EXPECT_EQ(tokens[2].type(), TokenType::Eof);
}