#ifndef VALUE_H
#define VALUE_H

#include <string>
#include <vector>
#include <memory>
#include <iostream>

class Environment;
class BlockNode;

// Represents runtime data types in our language
enum class ValueType {
    NUMBER,
    STRING,
    BOOLEAN,
    FUNCTION,
    NIL
};

// Represents a user-defined function object holding parameters, body, and closure
struct FunctionObject {
    std::string name;
    std::vector<std::string> parameters;
    std::shared_ptr<BlockNode> body;
    std::shared_ptr<Environment> closure;
};

// Represents any runtime value stored in variables or produced by expressions
struct Value {
    ValueType type;
    long long numberValue = 0;
    std::string stringValue;
    bool boolValue = false;
    std::shared_ptr<FunctionObject> funcValue = nullptr;

    Value() : type(ValueType::NIL) {}
    explicit Value(long long n) : type(ValueType::NUMBER), numberValue(n) {}
    explicit Value(std::string s) : type(ValueType::STRING), stringValue(std::move(s)) {}
    explicit Value(bool b) : type(ValueType::BOOLEAN), boolValue(b) {}
    explicit Value(std::shared_ptr<FunctionObject> fn) : type(ValueType::FUNCTION), funcValue(std::move(fn)) {}

    // Determines truthiness for conditionals
    bool isTruthy() const {
        if (type == ValueType::NIL) return false;
        if (type == ValueType::BOOLEAN) return boolValue;
        if (type == ValueType::NUMBER) return numberValue != 0;
        if (type == ValueType::STRING) return !stringValue.empty();
        if (type == ValueType::FUNCTION) return true;
        return false;
    }

    // Formats value for printing in the terminal
    std::string toString() const {
        if (type == ValueType::NUMBER) return std::to_string(numberValue);
        if (type == ValueType::STRING) return stringValue;
        if (type == ValueType::BOOLEAN) return boolValue ? "true" : "false";
        if (type == ValueType::FUNCTION) return "<fn " + (funcValue ? funcValue->name : "anonymous") + ">";
        return "nil";
    }
};

#endif
