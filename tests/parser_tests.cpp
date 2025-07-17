#include "lexer.h"
#include "parser.h"
#include <gtest/gtest.h>
#include <iostream>
#include <sstream>
#include <memory>

TEST(ParserTest, ParseSimpleFunction) {
    std::string code = R"(
        int main(void) {
            int x;
            x = 1 + 2;
            return x;
        }
    )";

    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse_program();

    ASSERT_TRUE(ast != nullptr);

    std::ostringstream oss;
    std::streambuf* old = std::cout.rdbuf(oss.rdbuf());
    ast->print();
    std::cout.rdbuf(old);

    std::string output = oss.str();

    ASSERT_NE(output.find("FunDeclaration(int main)"), std::string::npos);
    ASSERT_NE(output.find("VarDeclaration(int x)"), std::string::npos);
    ASSERT_NE(output.find("BinaryOp(=)"), std::string::npos);
    ASSERT_NE(output.find("BinaryOp(+)"), std::string::npos);
    ASSERT_NE(output.find("ReturnStmt"), std::string::npos);
}

TEST(ParserTest, SyntaxError) {
    std::string code = R"(
        int main(void) {
            int x
            return x;
        }
    )";

    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);

    testing::internal::CaptureStderr();
    auto ast = parser.parse_program();
    std::string err = testing::internal::GetCapturedStderr();

    ASSERT_NE(err.find("Syntax error"), std::string::npos);
}

TEST(ParserTest, ParseFunctionWithParameters) {
    std::string code = R"(
        int add(int a, int b) {
            return a + b;
        }

        int main(void) {
            int result;
            result = add(5, 3);
            return result;
        }
    )";

    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse_program();

    ASSERT_TRUE(ast != nullptr);

    std::ostringstream oss;
    std::streambuf* old = std::cout.rdbuf(oss.rdbuf());
    ast->print();
    std::cout.rdbuf(old);

    std::string output = oss.str();

    ASSERT_NE(output.find("FunDeclaration(int add)"), std::string::npos);
    ASSERT_NE(output.find("FunDeclaration(int main)"), std::string::npos);
}

// Conditionally compile main() only when this file is built standalone
#ifndef UNIT_TEST_MAIN
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();
    if (result == 0) {
        std::cout << "all tests passed" << std::endl;
    }
    return result;
}
#endif
