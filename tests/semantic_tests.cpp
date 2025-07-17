#include "semantic-analyzer.h"
#include "parser.h"
#include "lexer.h"
#include <gtest/gtest.h>

class SemanticAnalyzerTest : public ::testing::Test {
protected:
    std::unique_ptr<Program> parseProgram(const std::string& source) {
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        Parser parser(tokens);
        auto ast = parser.parse_program();
        return std::unique_ptr<Program>(dynamic_cast<Program*>(ast.release()));
    }

    SemanticAnalyzer analyzer;

    void SetUp() override {
        analyzer.reset_analysis();
    }
};

TEST_F(SemanticAnalyzerTest, ValidMainFunction) {
    std::string source = R"(
        int main(void) {
            return 0;
        }
    )";

    auto ast = parseProgram(source);
    ASSERT_TRUE(ast != nullptr);

    bool result = analyzer.analyze(*ast);
    EXPECT_TRUE(result);
    EXPECT_FALSE(analyzer.has_errors());
}

TEST_F(SemanticAnalyzerTest, MissingMainFunction) {
    std::string source = R"(
        int foo(void) {
            return 0;
        }
    )";

    auto ast = parseProgram(source);
    ASSERT_TRUE(ast != nullptr);

    bool result = analyzer.analyze(*ast);
    EXPECT_FALSE(result);
    EXPECT_TRUE(analyzer.has_errors());
}

TEST_F(SemanticAnalyzerTest, UndefinedVariable) {
    std::string source = R"(
        int main(void) {
            x = 5;
            return 0;
        }
    )";

    auto ast = parseProgram(source);
    ASSERT_TRUE(ast != nullptr);

    bool result = analyzer.analyze(*ast);
    EXPECT_FALSE(result);
    EXPECT_TRUE(analyzer.has_errors());
}

TEST_F(SemanticAnalyzerTest, VoidVariableError) {
    std::string source = R"(
        int main(void) {
            void x;
            return 0;
        }
    )";

    auto ast = parseProgram(source);
    ASSERT_TRUE(ast != nullptr);

    bool result = analyzer.analyze(*ast);
    EXPECT_FALSE(result);
    EXPECT_TRUE(analyzer.has_errors());
}

TEST_F(SemanticAnalyzerTest, FunctionCallValidation) {
    std::string source = R"(
        int main(void) {
            int x;
            x = 5;
            return x;
        }
    )";

    auto ast = parseProgram(source);
    ASSERT_TRUE(ast != nullptr);

    bool result = analyzer.analyze(*ast);
    EXPECT_TRUE(result);
    EXPECT_FALSE(analyzer.has_errors());
}

TEST_F(SemanticAnalyzerTest, BuiltinFunctions) {
    std::string source = R"(
        int main(void) {
            int x;
            x = input();
            output(x);
            return 0;
        }
    )";

    auto ast = parseProgram(source);
    ASSERT_TRUE(ast != nullptr);

    bool result = analyzer.analyze(*ast);
    EXPECT_TRUE(result);
    EXPECT_FALSE(analyzer.has_errors());
}

// Conditionally compile main() only when this file is built standalone
#ifndef UNIT_TEST_MAIN
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
#endif
