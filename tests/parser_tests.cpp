#include "lexer.h"
#include "parser.h"
#include <gtest/gtest.h>
#include <sstream>
#include <fstream>
#include <iostream>

int main(int argc, char **argv) {
    // Redirect stderr to /dev/null to suppress error output
    std::ofstream null_stream("/dev/null");
    // std::cerr.rdbuf(null_stream.rdbuf());

    ::testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();
    if (result == 0) {
        std::cout << "all tests passed" << std::endl;
    }
    return result;
}

TEST(ParserTest, ParseSimpleFunction) {
    std::string code = "int main() { int x; x = 1 + 2; return x; }";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse_program();
    ASSERT_TRUE(ast != nullptr);

    // Capture AST print output
    std::ostringstream oss;
    std::streambuf* old = std::cout.rdbuf(oss.rdbuf());
    ast->print();
    std::cout.rdbuf(old);

    std::string output = oss.str();
    // Check for key nodes
    ASSERT_NE(output.find("FunDeclaration(int main)"), std::string::npos);
    ASSERT_NE(output.find("VarDeclaration(int x)"), std::string::npos);
    ASSERT_NE(output.find("BinaryOp(=)"), std::string::npos);
    ASSERT_NE(output.find("BinaryOp(+)"), std::string::npos);
    ASSERT_NE(output.find("ReturnStmt"), std::string::npos);
}

TEST(ParserTest, SyntaxError) {
    std::string code = "int main( { int x; }"; // Missing ')'
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);

    testing::internal::CaptureStderr();
    auto ast = parser.parse_program();
    std::string err = testing::internal::GetCapturedStderr();

    ASSERT_NE(err.find("Syntax error"), std::string::npos);
}