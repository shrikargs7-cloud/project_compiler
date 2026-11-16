#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <string>
#include <unordered_map>
#include <memory>
#include <stdexcept>
#include "value.h"

// Symbol table that manages variable names, their values, and nested lexical scopes
class Environment : public std::enable_shared_from_this<Environment> {
private:
    std::unordered_map<std::string, Value> bindings;
    std::shared_ptr<Environment> enclosing; // Pointer to parent scope (nullptr if global)

public:
    Environment() : enclosing(nullptr) {}
    explicit Environment(std::shared_ptr<Environment> parent) : enclosing(std::move(parent)) {}

    // Declare a new variable in current scope
    void define(const std::string& name, const Value& val) {
        bindings[name] = val;
    }

    // Retrieve a variable value by looking up current scope and walking up the chain
    Value get(const std::string& name) {
        auto it = bindings.find(name);
        if (it != bindings.end()) {
            return it->second;
        }
        if (enclosing != nullptr) {
            return enclosing->get(name);
        }
        throw std::runtime_error("Undefined variable '" + name + "'");
    }

    // Assign to an existing variable
    void assign(const std::string& name, const Value& val) {
        auto it = bindings.find(name);
        if (it != bindings.end()) {
            it->second = val;
            return;
        }
        if (enclosing != nullptr) {
            enclosing->assign(name, val);
            return;
        }
        throw std::runtime_error("Undefined variable '" + name + "'");
    }
};

#endif
