#include "assembly-generator.h"
#include "ir-generator.h"
#include "parser.h"
#include "lexer.h"
#include "semantic-analyzer.h"
#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>

class AssemblyTest : public ::testing::Test {
protected:
    std::tuple<std::unique_ptr<Program>, std::shared_ptr<SemanticAnalyzer>>
    parseAndAnalyze(const std::string& source) {
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        Parser parser(tokens);
        auto ast = parser.parse_program();
        auto program = std::unique_ptr<Program>(dynamic_cast<Program*>(ast.release()));
        if (!program) {
            throw std::runtime_error("Parse error: AST root is not Program");
        }
        auto analyzer = std::make_shared<SemanticAnalyzer>();
        analyzer->analyze(*program);
        return std::make_tuple(std::move(program), analyzer);
    }

    void SetUp() override {
        std::filesystem::create_directory("test_output");
    }

    void TearDown() override {
        std::filesystem::remove_all("test_output");
    }
};

TEST_F(AssemblyTest, SimpleArithmetic) {
    std::string source = R"(
        int main(void) {
            int x;
            int y;
            int z;
            x = 5;
            y = 3;
            z = x + y;
            return z;
        }
    )";

    auto [program, analyzer] = parseAndAnalyze(source);
    ASSERT_TRUE(program != nullptr);

    IRGenerator ir_generator(analyzer.get());
    auto ir = ir_generator.generate(*program);

    AssemblyGenerator asm_generator("test_output/arithmetic.s");
    asm_generator.generate_from_ir(ir);

    std::ifstream asm_file("test_output/arithmetic.s");
    EXPECT_TRUE(asm_file.good());

    std::string line;
    bool found_add = false;
    bool found_main = false;

    while (std::getline(asm_file, line)) {
        if (line.find("add") != std::string::npos) {
            found_add = true;
        }
        if (line.find("main:") != std::string::npos) {
            found_main = true;
        }
    }

    EXPECT_TRUE(found_add);
    EXPECT_TRUE(found_main);
}

TEST_F(AssemblyTest, FunctionCall) {
    std::string source = R"(
        int add(int a, int b) {
            return a + b;
        }

        int main(void) {
            int result;
            result = add(5, 3);
            return result;
        }
    )";

    auto [program, analyzer] = parseAndAnalyze(source);
    ASSERT_TRUE(program != nullptr);

    IRGenerator ir_generator(analyzer.get());
    auto ir = ir_generator.generate(*program);

    AssemblyGenerator asm_generator("test_output/function_call.s");
    asm_generator.generate_from_ir(ir);

    std::ifstream asm_file("test_output/function_call.s");
    EXPECT_TRUE(asm_file.good());

    std::string line;
    bool found_call = false;

    while (std::getline(asm_file, line)) {
        if (line.find("call") != std::string::npos) {
            found_call = true;
            break;
        }
    }

    EXPECT_TRUE(found_call);
}

TEST_F(AssemblyTest, ControlFlow) {
    std::string source = R"(
        int main(void) {
            int x;
            x = 10;
            if (x > 5) {
                x = x - 1;
            }
            return x;
        }
    )";

    auto [program, analyzer] = parseAndAnalyze(source);
    ASSERT_TRUE(program != nullptr);

    IRGenerator ir_generator(analyzer.get());
    auto ir = ir_generator.generate(*program);

    AssemblyGenerator asm_generator("test_output/control_flow.s");
    asm_generator.generate_from_ir(ir);

    std::ifstream asm_file("test_output/control_flow.s");
    EXPECT_TRUE(asm_file.good());

    std::string line;
    bool found_jump = false;

    while (std::getline(asm_file, line)) {
        if (line.find("jz") != std::string::npos || line.find("jnz") != std::string::npos) {
            found_jump = true;
            break;
        }
    }

    EXPECT_TRUE(found_jump);
}

// Conditionally compile main() only when this file is built standalone
#ifndef UNIT_TEST_MAIN
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
#endif
