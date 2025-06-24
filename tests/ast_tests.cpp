#include "ast.h"
#include <gtest/gtest.h>
#include <sstream>
#include <iostream>

// Helper to capture std::cout output
std::string capturePrint(const ASTNode& node) {
    std::ostringstream oss;
    std::streambuf* old = std::cout.rdbuf(oss.rdbuf());
    node.print();
    std::cout.rdbuf(old);
    return oss.str();
}

TEST(ASTTest, ManualTreeVisualization) {
    // int x = 42;
    auto varDecl = std::make_unique<VarDeclaration>("int", "x");
    auto num = std::make_unique<Number>(42);
    auto assign = std::make_unique<BinaryOp>("=", std::make_unique<Variable>("x"), std::move(num));
    auto exprStmt = std::make_unique<BinaryOp>(";", std::move(assign), nullptr);

    auto compStmt = std::make_unique<CompoundStmt>();
    compStmt->locals.push_back(std::move(varDecl));
    compStmt->statements.push_back(std::move(exprStmt));

    auto func = std::make_unique<FunDeclaration>("int", "main");
    func->body = std::move(compStmt);

    // Program root
    auto program = std::make_unique<Program>();
    program->declarations.push_back(std::move(func));

    std::string output = capturePrint(*program);
    ASSERT_NE(output.find("FunDeclaration(int main)"), std::string::npos);
    ASSERT_NE(output.find("VarDeclaration(int x)"), std::string::npos);
    ASSERT_NE(output.find("Number(42)"), std::string::npos);
}

TEST(ASTTest, ErrorNodeVisualization) {
    auto error = std::make_unique<ErrorNode>("Unexpected token ';'");
    std::string output = capturePrint(*error);
    ASSERT_NE(output.find("ErrorNode(Unexpected token ';')"), std::string::npos);
}

TEST(ASTTest, IfWhileTreeVisualization) {
    // if (x) { return x; } else { return 0; }
    auto cond = std::make_unique<Variable>("x");
    auto thenStmt = std::make_unique<ReturnStmt>();
    thenStmt->expr = std::make_unique<Variable>("x");
    auto elseStmt = std::make_unique<ReturnStmt>();
    elseStmt->expr = std::make_unique<Number>(0);

    auto ifStmt = std::make_unique<IfStmt>();
    ifStmt->cond = std::move(cond);
    ifStmt->thenStmt = std::move(thenStmt);
    ifStmt->elseStmt = std::move(elseStmt);

    std::string output = capturePrint(*ifStmt);
    ASSERT_NE(output.find("IfStmt"), std::string::npos);
    ASSERT_NE(output.find("ReturnStmt"), std::string::npos);
}

TEST(ASTTest, VisualizeComplexFunction) {
    std::cout << "\nParsing and visualizing the following C-- function:\n"
              << "int main() {\n"
              << "  int x;\n"
              << "  x = 1 + 2;\n"
              << "  if (x > 0) {\n"
              << "    x = x - 1;\n"
              << "  } else {\n"
              << "    x = 0;\n"
              << "  }\n"
              << "  while (x) {\n"
              << "    x = x - 1;\n"
              << "  }\n"
              << "  return x;\n"
              << "}\n\n";

    // Variable declaration: int x;
    auto varDecl = std::make_unique<VarDeclaration>("int", "x");

    // Assignment: x = 1 + 2;
    auto assign1 = std::make_unique<BinaryOp>("=",
        std::make_unique<Variable>("x"),
        std::make_unique<BinaryOp>("+",
            std::make_unique<Number>(1),
            std::make_unique<Number>(2)
        )
    );

    // If-else: if (x > 0) { x = x - 1; } else { x = 0; }
    auto ifCond = std::make_unique<BinaryOp>(">",
        std::make_unique<Variable>("x"),
        std::make_unique<Number>(0)
    );
    auto thenAssign = std::make_unique<BinaryOp>("=",
        std::make_unique<Variable>("x"),
        std::make_unique<BinaryOp>("-",
            std::make_unique<Variable>("x"),
            std::make_unique<Number>(1)
        )
    );
    auto elseAssign = std::make_unique<BinaryOp>("=",
        std::make_unique<Variable>("x"),
        std::make_unique<Number>(0)
    );
    auto ifStmt = std::make_unique<IfStmt>();
    ifStmt->cond = std::move(ifCond);
    ifStmt->thenStmt = std::move(thenAssign);
    ifStmt->elseStmt = std::move(elseAssign);

    // While: while (x) { x = x - 1; }
    auto whileCond = std::make_unique<Variable>("x");
    auto whileBody = std::make_unique<BinaryOp>("=",
        std::make_unique<Variable>("x"),
        std::make_unique<BinaryOp>("-",
            std::make_unique<Variable>("x"),
            std::make_unique<Number>(1)
        )
    );
    auto whileStmt = std::make_unique<WhileStmt>();
    whileStmt->cond = std::move(whileCond);
    whileStmt->body = std::move(whileBody);

    // Return: return x;
    auto retStmt = std::make_unique<ReturnStmt>();
    retStmt->expr = std::make_unique<Variable>("x");

    // Compound statement (function body)
    auto compStmt = std::make_unique<CompoundStmt>();
    compStmt->locals.push_back(std::move(varDecl));
    compStmt->statements.push_back(std::move(assign1));
    compStmt->statements.push_back(std::move(ifStmt));
    compStmt->statements.push_back(std::move(whileStmt));
    compStmt->statements.push_back(std::move(retStmt));

    // Function: int main() { ... }
    auto func = std::make_unique<FunDeclaration>("int", "main");
    func->body = std::move(compStmt);

    // Program root
    auto program = std::make_unique<Program>();
    program->declarations.push_back(std::move(func));

    std::string output = capturePrint(*program);
    std::cout << output; // For manual inspection

    // Check for key nodes in the output
    ASSERT_NE(output.find("FunDeclaration(int main)"), std::string::npos);
    ASSERT_NE(output.find("VarDeclaration(int x)"), std::string::npos);
    ASSERT_NE(output.find("BinaryOp(=)"), std::string::npos);
    ASSERT_NE(output.find("BinaryOp(+)"), std::string::npos);
    ASSERT_NE(output.find("IfStmt"), std::string::npos);
    ASSERT_NE(output.find("WhileStmt"), std::string::npos);
    ASSERT_NE(output.find("ReturnStmt"), std::string::npos);
}