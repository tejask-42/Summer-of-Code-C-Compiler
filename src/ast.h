#pragma once
#include <string>
#include <vector>
#include <memory>
#include <iostream>

class Visitor;

// Base AST node
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void print(int indent = 0) const = 0;
    virtual void accept(Visitor& visitor) = 0;
protected:
    void printIndent(int indent) const {
        for (int i = 0; i < indent; ++i) std::cout << "  ";
    }
};

class VarDeclaration : public ASTNode {
public:
    std::string type, name;
    int arraySize; 
    VarDeclaration(const std::string& t, const std::string& n, int size = -1)
        : type(t), name(n), arraySize(size) {}
    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "VarDeclaration(" << type << " " << name;
        if (arraySize != -1) std::cout << "[" << arraySize << "]";
        std::cout << ")\n";
    }
    void accept(Visitor& visitor) override;
};

class Parameter : public ASTNode {
public:
    std::string type, name;
    bool isArray;
    Parameter(const std::string& t, const std::string& n, bool arr = false)
        : type(t), name(n), isArray(arr) {}
    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "Parameter(" << type << " " << name;
        if (isArray) std::cout << "[]";
        std::cout << ")\n";
    }
    void accept(Visitor& visitor) override;
};

class FunDeclaration : public ASTNode {
public:
    std::string type, name;
    std::vector<std::unique_ptr<Parameter>> params;
    std::unique_ptr<ASTNode> body; 
    FunDeclaration(const std::string& t, const std::string& n)
        : type(t), name(n) {}
    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "FunDeclaration(" << type << " " << name << ")\n";
        printIndent(indent + 1); std::cout << "Parameters:\n";
        for (const auto& p : params) p->print(indent + 2);
        printIndent(indent + 1); std::cout << "Body:\n";
        if (body) body->print(indent + 2);
    }
    void accept(Visitor& visitor) override;
};

class CompoundStmt : public ASTNode {
public:
    std::vector<std::unique_ptr<VarDeclaration>> locals;
    std::vector<std::unique_ptr<ASTNode>> statements;
    void print(int indent = 0) const override {
        printIndent(indent); std::cout << "CompoundStmt\n";
        printIndent(indent + 1); std::cout << "Locals:\n";
        for (const auto& l : locals) l->print(indent + 2);
        printIndent(indent + 1); std::cout << "Statements:\n";
        for (const auto& s : statements) s->print(indent + 2);
    }
    void accept(Visitor& visitor) override;
};

class IfStmt : public ASTNode {
public:
    std::unique_ptr<ASTNode> cond, thenStmt, elseStmt;
    void print(int indent = 0) const override {
        printIndent(indent); std::cout << "IfStmt\n";
        printIndent(indent + 1); std::cout << "Condition:\n";
        if (cond) cond->print(indent + 2);
        printIndent(indent + 1); std::cout << "Then:\n";
        if (thenStmt) thenStmt->print(indent + 2);
        if (elseStmt) {
            printIndent(indent + 1); std::cout << "Else:\n";
            elseStmt->print(indent + 2);
        }
    }
    void accept(Visitor& visitor) override;
};

class WhileStmt : public ASTNode {
public:
    std::unique_ptr<ASTNode> cond, body;
    void print(int indent = 0) const override {
        printIndent(indent); std::cout << "WhileStmt\n";
        printIndent(indent + 1); std::cout << "Condition:\n";
        if (cond) cond->print(indent + 2);
        printIndent(indent + 1); std::cout << "Body:\n";
        if (body) body->print(indent + 2);
    }
    void accept(Visitor& visitor) override;
};

class ReturnStmt : public ASTNode {
public:
    std::unique_ptr<ASTNode> expr; // nullptr if just 'return;'
    void print(int indent = 0) const override {
        printIndent(indent); std::cout << "ReturnStmt\n";
        if (expr) expr->print(indent + 1);
    }
    void accept(Visitor& visitor) override;
};

class BinaryOp : public ASTNode {
public:
    std::string op;
    std::unique_ptr<ASTNode> left, right;
    BinaryOp(const std::string& o, std::unique_ptr<ASTNode> l, std::unique_ptr<ASTNode> r)
        : op(o), left(std::move(l)), right(std::move(r)) {}
    void print(int indent = 0) const override {
        printIndent(indent); std::cout << "BinaryOp(" << op << ")\n";
        if (left) left->print(indent + 1);
        if (right) right->print(indent + 1);
    }
    void accept(Visitor& visitor) override;
};

class UnaryOp : public ASTNode {
public:
    std::string op;
    std::unique_ptr<ASTNode> operand;
    UnaryOp(const std::string& o, std::unique_ptr<ASTNode> e)
        : op(o), operand(std::move(e)) {}
    void print(int indent = 0) const override {
        printIndent(indent); std::cout << "UnaryOp(" << op << ")\n";
        if (operand) operand->print(indent + 1);
    }
    void accept(Visitor& visitor) override;
};

class Variable : public ASTNode {
public:
    std::string name;
    std::unique_ptr<ASTNode> index; 
    Variable(const std::string& n, std::unique_ptr<ASTNode> idx = nullptr)
        : name(n), index(std::move(idx)) {}
    void print(int indent = 0) const override {
        printIndent(indent); std::cout << "Variable(" << name << ")\n";
        if (index) {
            printIndent(indent + 1); std::cout << "Index:\n";
            index->print(indent + 2);
        }
    }
    void accept(Visitor& visitor) override;
};

class Call : public ASTNode {
public:
    std::string name;
    std::vector<std::unique_ptr<ASTNode>> args;
    Call(const std::string& n) : name(n) {}
    void print(int indent = 0) const override {
        printIndent(indent); std::cout << "Call(" << name << ")\n";
        for (const auto& a : args) a->print(indent + 1);
    }
    void accept(Visitor& visitor) override;
};

class Number : public ASTNode {
public:
    int value;
    Number(int v) : value(v) {}
    void print(int indent = 0) const override {
        printIndent(indent); std::cout << "Number(" << value << ")\n";
    }
    void accept(Visitor& visitor) override;
};

class ErrorNode : public ASTNode {
public:
    std::string message;
    ErrorNode(const std::string& msg) : message(msg) {}
    void print(int indent = 0) const override {
        printIndent(indent); std::cout << "ErrorNode(" << message << ")\n";
    }
    void accept(Visitor& visitor) override {}
};

class Program : public ASTNode {
public:
    std::vector<std::unique_ptr<ASTNode>> declarations;
    void print(int indent = 0) const override {
        printIndent(indent); std::cout << "Program\n";
        for (const auto& decl : declarations)
            decl->print(indent + 1);
    }
    void accept(Visitor& visitor) override;
};

class Visitor {
public:
    virtual void visit(VarDeclaration&) = 0;
    virtual void visit(Parameter&) = 0;
    virtual void visit(FunDeclaration&) = 0;
    virtual void visit(CompoundStmt&) = 0;
    virtual void visit(IfStmt&) = 0;
    virtual void visit(WhileStmt&) = 0;
    virtual void visit(ReturnStmt&) = 0;
    virtual void visit(BinaryOp&) = 0;
    virtual void visit(UnaryOp&) = 0;
    virtual void visit(Variable&) = 0;
    virtual void visit(Call&) = 0;
    virtual void visit(Number&) = 0;
    virtual void visit(Program&) = 0;
    virtual ~Visitor() = default;
};

inline void VarDeclaration::accept(Visitor& v) { v.visit(*this); }
inline void Parameter::accept(Visitor& v) { v.visit(*this); }
inline void FunDeclaration::accept(Visitor& v) { v.visit(*this); }
inline void CompoundStmt::accept(Visitor& v) { v.visit(*this); }
inline void IfStmt::accept(Visitor& v) { v.visit(*this); }
inline void WhileStmt::accept(Visitor& v) { v.visit(*this); }
inline void ReturnStmt::accept(Visitor& v) { v.visit(*this); }
inline void BinaryOp::accept(Visitor& v) { v.visit(*this); }
inline void UnaryOp::accept(Visitor& v) { v.visit(*this); }
inline void Variable::accept(Visitor& v) { v.visit(*this); }
inline void Call::accept(Visitor& v) { v.visit(*this); }
inline void Number::accept(Visitor& v) { v.visit(*this); }
inline void Program::accept(Visitor& v) { v.visit(*this); }