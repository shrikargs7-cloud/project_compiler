#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <memory>
#include "ast.h"
#include "environment.h"
#include "value.h"

// The execution engine: traverses the AST, manages runtime state, and evaluates instructions
class Interpreter {
private:
    std::shared_ptr<Environment> globalEnv;
    std::shared_ptr<Environment> currentEnv;

    Value evaluate(const ASTNode* node);
    void execute(const ASTNode* node);

public:
    Interpreter();
    void interpret(const ProgramNode* program);
    void executeStatement(const ASTNode* stmt);
};

#endif
