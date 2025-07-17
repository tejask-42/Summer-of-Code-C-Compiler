#include "ir-generator.h"
#include "ir-optimizer.h"
#include "cfg.h"
#include "parser.h"
#include "lexer.h"
#include "semantic-analyzer.h"
#include <memory>
#include <tuple>
#include <string>
#include <gtest/gtest.h>
#include <iostream>

class IRTest : public ::testing::Test {
protected:
    std::tuple<std::unique_ptr<Program>, std::shared_ptr<SemanticAnalyzer>> parseAndAnalyze(const std::string& source) {
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        Parser parser(tokens);
        auto ast = parser.parse_program();

        auto program = std::unique_ptr<Program>(dynamic_cast<Program*>(ast.release()));
        if (!program) {
            std::cerr << "Parse error: AST root is not Program\n";
            exit(1);
        }

        auto analyzer = std::make_shared<SemanticAnalyzer>();
        analyzer->analyze(*program);

        return std::make_tuple(std::move(program), analyzer);
    }
};

TEST_F(IRTest, BasicArithmetic) {
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

    IRGenerator generator(analyzer.get());
    auto ir = generator.generate(*program);

    EXPECT_GT(ir.size(), 0);

    bool found_add = false;
    for (const auto& instr : ir) {
        if (instr.op == OpCode::ADD) {
            found_add = true;
            break;
        }
    }
    EXPECT_TRUE(found_add);
}

TEST_F(IRTest, ControlFlow) {
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

    IRGenerator generator(analyzer.get());
    auto ir = generator.generate(*program);

    EXPECT_GT(ir.size(), 0);

    bool found_if = false;
    bool found_label = false;
    for (const auto& instr : ir) {
        if (instr.op == OpCode::IF_FALSE) found_if = true;
        if (instr.op == OpCode::LABEL) found_label = true;
    }
    EXPECT_TRUE(found_if);
    EXPECT_TRUE(found_label);
}

TEST_F(IRTest, Optimization) {
    std::string source = R"(
        int main(void) {
            int x;
            int y;
            x = 5;
            x = x + 3;
            y = x;
            y = y * 1;
            return y;
        }
    )";

    auto [program, analyzer] = parseAndAnalyze(source);
    ASSERT_TRUE(program != nullptr);

    IRGenerator generator(analyzer.get());
    auto ir = generator.generate(*program);

    IROptimizer optimizer;
    auto optimized = optimizer.optimize(ir);

    EXPECT_LE(optimized.size(), ir.size());
}

TEST_F(IRTest, CFGConstruction) {
    std::string source = R"(
        int main(void) {
            int x;
            x = 10;
            while (x > 0) {
                x = x - 1;
            }
            return x;
        }
    )";

    auto [program, analyzer] = parseAndAnalyze(source);
    ASSERT_TRUE(program != nullptr);

    IRGenerator generator(analyzer.get());
    auto ir = generator.generate(*program);

    ControlFlowGraph cfg;
    cfg.build_from_ir(ir);

    EXPECT_GT(cfg.get_blocks().size(), 0);
    EXPECT_TRUE(cfg.get_entry_block() != nullptr);
}

// Conditionally compile main() only when this file is built standalone
#ifndef UNIT_TEST_MAIN
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
#endif
