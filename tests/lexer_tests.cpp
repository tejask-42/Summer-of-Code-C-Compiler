#include "lexer.h"
#include <gtest/gtest.h>

TEST(LexerTest, BasicTokenization) {
    std::string code = "int main(void) { return 0; }";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();

    EXPECT_GT(tokens.size(), 0);
    EXPECT_EQ(tokens[0].type(), TokenType::Int);
    EXPECT_EQ(tokens[1].type(), TokenType::Identifier);
    EXPECT_EQ(tokens[1].value(), "main");
}

TEST(LexerTest, NumberTokenization) {
    std::string code = "42 123 0";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();

    EXPECT_EQ(tokens[0].type(), TokenType::Number);
    EXPECT_EQ(tokens[0].value(), "42");
    EXPECT_EQ(tokens[1].type(), TokenType::Number);
    EXPECT_EQ(tokens[1].value(), "123");
    EXPECT_EQ(tokens[2].type(), TokenType::Number);
    EXPECT_EQ(tokens[2].value(), "0");
}

TEST(LexerTest, OperatorTokenization) {
    std::string code = "+ - * / = == != < > <= >=";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();

    EXPECT_EQ(tokens[0].type(), TokenType::Plus);
    EXPECT_EQ(tokens[1].type(), TokenType::Minus);
    EXPECT_EQ(tokens[2].type(), TokenType::Star);
    EXPECT_EQ(tokens[3].type(), TokenType::Slash);
    EXPECT_EQ(tokens[4].type(), TokenType::Equal);
    EXPECT_EQ(tokens[5].type(), TokenType::EqualEqual);
    EXPECT_EQ(tokens[6].type(), TokenType::NotEqual);
    EXPECT_EQ(tokens[7].type(), TokenType::Less);
    EXPECT_EQ(tokens[8].type(), TokenType::Greater);
    EXPECT_EQ(tokens[9].type(), TokenType::LessEqual);
    EXPECT_EQ(tokens[10].type(), TokenType::GreaterEqual);
}

// Conditionally compile main() only when this file is built standalone
#ifndef UNIT_TEST_MAIN
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
#endif
