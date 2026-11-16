#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <memory>
#include <iostream>

// Base class for all Abstract Syntax Tree (AST) nodes
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void print(int indent = 0) const = 0;
};

// Represents numeric literals (e.g. 100, 25)
class NumberNode : public ASTNode {
public:
    long long value;
    explicit NumberNode(long long val) : value(val) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent * 2, ' ') << "└── Number: " << value << "\n";
    }
};

// Represents string literals (e.g. "Total calculated score:")
class StringNode : public ASTNode {
public:
    std::string value;
    explicit StringNode(std::string val) : value(std::move(val)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent * 2, ' ') << "└── String: \"" << value << "\"\n";
    }
};

// Represents accessing a variable by name (e.g. `score`)
class VariableAccessNode : public ASTNode {
public:
    std::string name;
    explicit VariableAccessNode(std::string n) : name(std::move(n)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent * 2, ' ') << "└── VariableAccess: " << name << "\n";
    }
};

// Represents binary operations (+, -, *, /, <, <=, >, >=, ==, !=)
class BinaryOpNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> left;
    std::string op;
    std::unique_ptr<ASTNode> right;

    BinaryOpNode(std::unique_ptr<ASTNode> l, std::string o, std::unique_ptr<ASTNode> r)
        : left(std::move(l)), op(std::move(o)), right(std::move(r)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent * 2, ' ') << "├── BinaryOp: '" << op << "'\n";
        if (left) left->print(indent + 1);
        if (right) right->print(indent + 1);
    }
};

// Represents a variable declaration (e.g. `let score = 100;`)
class VarDeclNode : public ASTNode {
public:
    std::string name;
    std::unique_ptr<ASTNode> initializer;

    VarDeclNode(std::string n, std::unique_ptr<ASTNode> init)
        : name(std::move(n)), initializer(std::move(init)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent * 2, ' ') << "├── VarDecl: 'let " << name << " ='\n";
        if (initializer) {
            initializer->print(indent + 1);
        }
    }
};

// Represents a variable reassignment (e.g. `score = score - 1;`)
class AssignmentNode : public ASTNode {
public:
    std::string name;
    std::unique_ptr<ASTNode> value;

    AssignmentNode(std::string n, std::unique_ptr<ASTNode> v)
        : name(std::move(n)), value(std::move(v)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent * 2, ' ') << "├── Assignment: '" << name << " ='\n";
        if (value) {
            value->print(indent + 1);
        }
    }
};

// Represents a print call (e.g. `print(total);`)
class PrintNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> expression;

    explicit PrintNode(std::unique_ptr<ASTNode> expr) : expression(std::move(expr)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent * 2, ' ') << "├── PrintStatement\n";
        if (expression) {
            expression->print(indent + 1);
        }
    }
};

// Represents a block of code (e.g. `{ ... }`)
class BlockNode : public ASTNode {
public:
    std::vector<std::unique_ptr<ASTNode>> statements;

    void addStatement(std::unique_ptr<ASTNode> stmt) {
        statements.push_back(std::move(stmt));
    }

    void print(int indent = 0) const override {
        std::cout << std::string(indent * 2, ' ') << "├── Block\n";
        for (const auto& stmt : statements) {
            stmt->print(indent + 1);
        }
    }
};

// Represents an if-statement (e.g. `if (total > 50) { ... }`)
class IfNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> condition;
    std::unique_ptr<ASTNode> thenBranch;
    std::unique_ptr<ASTNode> elseBranch;

    IfNode(std::unique_ptr<ASTNode> cond, std::unique_ptr<ASTNode> thenB, std::unique_ptr<ASTNode> elseB = nullptr)
        : condition(std::move(cond)), thenBranch(std::move(thenB)), elseBranch(std::move(elseB)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent * 2, ' ') << "├── IfStatement\n";
        std::cout << std::string((indent + 1) * 2, ' ') << "├── Condition:\n";
        if (condition) condition->print(indent + 2);
        std::cout << std::string((indent + 1) * 2, ' ') << "├── Then Branch:\n";
        if (thenBranch) thenBranch->print(indent + 2);
        if (elseBranch) {
            std::cout << std::string((indent + 1) * 2, ' ') << "└── Else Branch:\n";
            elseBranch->print(indent + 2);
        }
    }
};

// Represents a while-loop (e.g. `while (x > 0) { ... }`)
class WhileNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> condition;
    std::unique_ptr<ASTNode> body;

    WhileNode(std::unique_ptr<ASTNode> cond, std::unique_ptr<ASTNode> b)
        : condition(std::move(cond)), body(std::move(b)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent * 2, ' ') << "├── WhileLoop\n";
        std::cout << std::string((indent + 1) * 2, ' ') << "├── Condition:\n";
        if (condition) condition->print(indent + 2);
        std::cout << std::string((indent + 1) * 2, ' ') << "└── Body:\n";
        if (body) body->print(indent + 2);
    }
};

// Represents a function declaration: `fn add(a, b) { ... }`
class FunctionDeclNode : public ASTNode {
public:
    std::string name;
    std::vector<std::string> parameters;
    std::shared_ptr<BlockNode> body; // shared ownership so runtime function values can hold it

    FunctionDeclNode(std::string n, std::vector<std::string> params, std::shared_ptr<BlockNode> b)
        : name(std::move(n)), parameters(std::move(params)), body(std::move(b)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent * 2, ' ') << "├── FunctionDecl: 'fn " << name << "(";
        for (size_t i = 0; i < parameters.size(); ++i) {
            std::cout << parameters[i] << (i + 1 < parameters.size() ? ", " : "");
        }
        std::cout << ")'\n";
        if (body) body->print(indent + 1);
    }
};

// Represents a return statement: `return expr;`
class ReturnNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> expression;

    explicit ReturnNode(std::unique_ptr<ASTNode> expr) : expression(std::move(expr)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent * 2, ' ') << "├── ReturnStatement\n";
        if (expression) expression->print(indent + 1);
    }
};

// Represents a function call: `add(10, 20)`
class CallNode : public ASTNode {
public:
    std::string callee;
    std::vector<std::unique_ptr<ASTNode>> arguments;

    CallNode(std::string name, std::vector<std::unique_ptr<ASTNode>> args)
        : callee(std::move(name)), arguments(std::move(args)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent * 2, ' ') << "├── Call: '" << callee << "(...)'\n";
        for (const auto& arg : arguments) {
            arg->print(indent + 1);
        }
    }
};

// Root node representing the entire parsed program
class ProgramNode : public ASTNode {
public:
    std::vector<std::unique_ptr<ASTNode>> statements;

    void addStatement(std::unique_ptr<ASTNode> stmt) {
        statements.push_back(std::move(stmt));
    }

    void print(int indent = 0) const override {
        std::cout << "\033[94mProgram AST Root:\033[0m\n";
        for (const auto& stmt : statements) {
            stmt->print(indent + 1);
        }
    }
};

#endif
