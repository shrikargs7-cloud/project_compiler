#include "interpreter.h"
#include <iostream>
#include <stdexcept>

// Exception type used to implement immediate function returns
struct ReturnSignal {
    Value value;
};

Interpreter::Interpreter() {
    globalEnv = std::make_shared<Environment>();
    currentEnv = globalEnv;
}

Value Interpreter::evaluate(const ASTNode* node) {
    if (!node) return Value();

    // 1. Number Literal
    if (auto num = dynamic_cast<const NumberNode*>(node)) {
        return Value(num->value);
    }

    // 2. String Literal
    if (auto str = dynamic_cast<const StringNode*>(node)) {
        return Value(str->value);
    }

    // 3. Variable Access
    if (auto var = dynamic_cast<const VariableAccessNode*>(node)) {
        return currentEnv->get(var->name);
    }

    // 4. Function Call: `callee(arg1, arg2, ...)`
    if (auto call = dynamic_cast<const CallNode*>(node)) {
        Value calleeVal = currentEnv->get(call->callee);
        if (calleeVal.type != ValueType::FUNCTION || !calleeVal.funcValue) {
            throw std::runtime_error("'" + call->callee + "' is not a function");
        }

        const auto& fn = calleeVal.funcValue;
        if (call->arguments.size() != fn->parameters.size()) {
            throw std::runtime_error("Function '" + fn->name + "' expected " + 
                                     std::to_string(fn->parameters.size()) + " arguments, got " +
                                     std::to_string(call->arguments.size()));
        }

        // Evaluate arguments in the caller's scope
        std::vector<Value> evaluatedArgs;
        for (const auto& arg : call->arguments) {
            evaluatedArgs.push_back(evaluate(arg.get()));
        }

        // Create new environment scope with closure as parent (lexical scoping)
        auto callEnv = std::make_shared<Environment>(fn->closure);
        for (size_t i = 0; i < fn->parameters.size(); ++i) {
            callEnv->define(fn->parameters[i], evaluatedArgs[i]);
        }

        // Execute function body in the new scope
        auto savedEnv = currentEnv;
        currentEnv = callEnv;
        Value returnValue;

        try {
            for (const auto& stmt : fn->body->statements) {
                execute(stmt.get());
            }
        } catch (const ReturnSignal& sig) {
            returnValue = sig.value;
        }

        currentEnv = savedEnv;
        return returnValue;
    }

    // 5. Binary Operations (+, -, *, /, ==, !=, <, <=, >, >=)
    if (auto bin = dynamic_cast<const BinaryOpNode*>(node)) {
        Value left = evaluate(bin->left.get());
        Value right = evaluate(bin->right.get());

        // Arithmetic on numbers
        if (left.type == ValueType::NUMBER && right.type == ValueType::NUMBER) {
            if (bin->op == "+") return Value(left.numberValue + right.numberValue);
            if (bin->op == "-") return Value(left.numberValue - right.numberValue);
            if (bin->op == "*") return Value(left.numberValue * right.numberValue);
            if (bin->op == "/") {
                if (right.numberValue == 0) {
                    throw std::runtime_error("Division by zero error");
                }
                return Value(left.numberValue / right.numberValue);
            }

            // Numeric comparisons
            if (bin->op == "==") return Value(left.numberValue == right.numberValue);
            if (bin->op == "!=") return Value(left.numberValue != right.numberValue);
            if (bin->op == "<")  return Value(left.numberValue < right.numberValue);
            if (bin->op == "<=") return Value(left.numberValue <= right.numberValue);
            if (bin->op == ">")  return Value(left.numberValue > right.numberValue);
            if (bin->op == ">=") return Value(left.numberValue >= right.numberValue);
        }

        // String operations
        if (left.type == ValueType::STRING || right.type == ValueType::STRING) {
            if (bin->op == "+") {
                return Value(left.toString() + right.toString());
            }
            if (bin->op == "==") {
                return Value(left.toString() == right.toString());
            }
            if (bin->op == "!=") {
                return Value(left.toString() != right.toString());
            }
        }

        // Boolean equality
        if (left.type == ValueType::BOOLEAN && right.type == ValueType::BOOLEAN) {
            if (bin->op == "==") return Value(left.boolValue == right.boolValue);
            if (bin->op == "!=") return Value(left.boolValue != right.boolValue);
        }

        throw std::runtime_error("Unsupported operand types for operator '" + bin->op + "'");
    }

    throw std::runtime_error("Unknown expression node in evaluation");
}

void Interpreter::execute(const ASTNode* node) {
    if (!node) return;

    // 1. Function Declaration (`fn name(...) { ... }`)
    if (auto fnDecl = dynamic_cast<const FunctionDeclNode*>(node)) {
        auto fnObj = std::make_shared<FunctionObject>();
        fnObj->name = fnDecl->name;
        fnObj->parameters = fnDecl->parameters;
        fnObj->body = fnDecl->body;
        fnObj->closure = currentEnv; // Captures current scope

        currentEnv->define(fnDecl->name, Value(fnObj));
        return;
    }

    // 2. Return Statement (`return expr;`)
    if (auto ret = dynamic_cast<const ReturnNode*>(node)) {
        Value val = ret->expression ? evaluate(ret->expression.get()) : Value();
        throw ReturnSignal{val};
    }

    // 3. Variable Declaration (`let name = expr;`)
    if (auto decl = dynamic_cast<const VarDeclNode*>(node)) {
        Value val = evaluate(decl->initializer.get());
        currentEnv->define(decl->name, val);
        return;
    }

    // 4. Variable Assignment (`name = expr;`)
    if (auto assign = dynamic_cast<const AssignmentNode*>(node)) {
        Value val = evaluate(assign->value.get());
        currentEnv->assign(assign->name, val);
        return;
    }

    // 5. Print Statement (`print(expr);`)
    if (auto prt = dynamic_cast<const PrintNode*>(node)) {
        Value val = evaluate(prt->expression.get());
        std::cout << val.toString() << "\n";
        return;
    }

    // 6. Block Statement (`{ ... }`)
    if (auto block = dynamic_cast<const BlockNode*>(node)) {
        auto previousEnv = currentEnv;
        currentEnv = std::make_shared<Environment>(previousEnv);

        for (const auto& stmt : block->statements) {
            execute(stmt.get());
        }

        currentEnv = previousEnv;
        return;
    }

    // 7. If Statement (`if (cond) { ... } else { ... }`)
    if (auto ifStmt = dynamic_cast<const IfNode*>(node)) {
        Value conditionVal = evaluate(ifStmt->condition.get());
        if (conditionVal.isTruthy()) {
            execute(ifStmt->thenBranch.get());
        } else if (ifStmt->elseBranch) {
            execute(ifStmt->elseBranch.get());
        }
        return;
    }

    // 8. While Loop (`while (cond) { ... }`)
    if (auto whileStmt = dynamic_cast<const WhileNode*>(node)) {
        while (evaluate(whileStmt->condition.get()).isTruthy()) {
            execute(whileStmt->body.get());
        }
        return;
    }

    // Default expression statement evaluation
    evaluate(node);
}

void Interpreter::executeStatement(const ASTNode* stmt) {
    execute(stmt);
}

void Interpreter::interpret(const ProgramNode* program) {
    if (!program) return;

    try {
        for (const auto& stmt : program->statements) {
            execute(stmt.get());
        }
    } catch (const std::exception& e) {
        std::cerr << "\033[91m[Runtime Error]\033[0m " << e.what() << "\n";
    }
}
