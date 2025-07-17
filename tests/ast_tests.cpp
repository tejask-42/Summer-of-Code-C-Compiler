#include "ast.h"
#include <gtest/gtest.h>

TEST(ASTTest, NumberNodeCreation) {
    auto num = std::make_unique<Number>(42);
    EXPECT_EQ(num->value, 42);
}

TEST(ASTTest, VariableNodeCreation) {
    auto var = std::make_unique<Variable>("x");
    EXPECT_EQ(var->name, "x");
}

TEST(ASTTest, BinaryOpNodeCreation) {
    auto left = std::make_unique<Number>(5);
    auto right = std::make_unique<Number>(3);
    auto binop = std::make_unique<BinaryOp>("+", std::move(left), std::move(right));
    
    EXPECT_EQ(binop->op, "+");
    EXPECT_TRUE(binop->left != nullptr);
    EXPECT_TRUE(binop->right != nullptr);
}

TEST(ASTTest, FunctionDeclarationCreation) {
    auto func = std::make_unique<FunDeclaration>(
        "int", "main", std::vector<std::unique_ptr<Parameter>>(), nullptr);
    EXPECT_EQ(func->name, "main");
    EXPECT_EQ(func->return_type, "int");
}

// Conditionally compile main() only when this file is built standalone
#ifndef UNIT_TEST_MAIN
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
#endif
