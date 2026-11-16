#include "parser.h"
#include <iostream>
#include <stdexcept>

Parser::Parser(std::vector<Token> tokens)
    : tokens(std::move(tokens)), current(0) {}

const Token& Parser::peek() const {
    return tokens[current];
}

const Token& Parser::previous() const {
    return tokens[current - 1];
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::END_OF_FILE;
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

Token Parser::advance() {
    if (!isAtEnd()) current++;
    return previous();
}

Token Parser::consume(TokenType type, const std::string& errorMessage) {
    if (check(type)) return advance();

    const Token& tok = peek();
    std::cerr << "\033[91m[Parser Error]\033[0m " << errorMessage 
              << " at line " << tok.line << ", col " << tok.column 
              << " (got '" << tok.lexeme << "')\n";
    throw std::runtime_error(errorMessage);
}

std::unique_ptr<ProgramNode> Parser::parse() {
    auto program = std::make_unique<ProgramNode>();

    while (!isAtEnd()) {
        try {
            auto stmt = parseStatement();
            if (stmt) {
                program->addStatement(std::move(stmt));
            }
        } catch (const std::exception& e) {
            // Panic mode recovery: synchronize to next semicolon
            while (!isAtEnd() && peek().type != TokenType::SEMICOLON) {
                advance();
            }
            if (!isAtEnd()) advance(); // consume ';'
        }
    }

    return program;
}

std::unique_ptr<ASTNode> Parser::parseStatement() {
    if (match(TokenType::KEYWORD_FN)) {
        return parseFunctionDeclaration();
    }
    if (match(TokenType::KEYWORD_RETURN)) {
        return parseReturnStatement();
    }
    if (match(TokenType::KEYWORD_LET)) {
        return parseVarDeclaration();
    }
    if (match(TokenType::KEYWORD_PRINT)) {
        return parsePrintStatement();
    }
    if (match(TokenType::KEYWORD_IF)) {
        return parseIfStatement();
    }
    if (match(TokenType::KEYWORD_WHILE)) {
        return parseWhileStatement();
    }
    if (match(TokenType::LBRACE)) {
        return parseBlock();
    }

    // Variable assignment: identifier '=' expression ';'
    if (check(TokenType::IDENTIFIER) && current + 1 < tokens.size() && tokens[current + 1].type == TokenType::EQUALS) {
        Token nameToken = advance();
        advance(); // consume '='
        auto val = parseExpression();
        consume(TokenType::SEMICOLON, "Expected ';' after assignment");
        return std::make_unique<AssignmentNode>(nameToken.lexeme, std::move(val));
    }

    // Default expression statement (e.g. `call();` or `score + 5;`)
    auto expr = parseExpression();
    consume(TokenType::SEMICOLON, "Expected ';' after expression statement");
    return expr;
}

std::unique_ptr<ASTNode> Parser::parseFunctionDeclaration() {
    Token name = consume(TokenType::IDENTIFIER, "Expected function name after 'fn'");
    consume(TokenType::LPAREN, "Expected '(' after function name");

    std::vector<std::string> parameters;
    if (!check(TokenType::RPAREN)) {
        do {
            Token param = consume(TokenType::IDENTIFIER, "Expected parameter name");
            parameters.push_back(param.lexeme);
        } while (match(TokenType::COMMA));
    }
    consume(TokenType::RPAREN, "Expected ')' after parameters");

    consume(TokenType::LBRACE, "Expected '{' before function body");
    auto body = parseBlock();

    return std::make_unique<FunctionDeclNode>(name.lexeme, std::move(parameters), std::move(body));
}

std::unique_ptr<ASTNode> Parser::parseReturnStatement() {
    std::unique_ptr<ASTNode> expr = nullptr;
    if (!check(TokenType::SEMICOLON)) {
        expr = parseExpression();
    }
    consume(TokenType::SEMICOLON, "Expected ';' after return value");
    return std::make_unique<ReturnNode>(std::move(expr));
}

std::unique_ptr<ASTNode> Parser::parseVarDeclaration() {
    Token nameToken = consume(TokenType::IDENTIFIER, "Expected variable name after 'let'");
    consume(TokenType::EQUALS, "Expected '=' after variable name");
    auto initializer = parseExpression();
    consume(TokenType::SEMICOLON, "Expected ';' after variable declaration");

    return std::make_unique<VarDeclNode>(nameToken.lexeme, std::move(initializer));
}

std::unique_ptr<ASTNode> Parser::parsePrintStatement() {
    consume(TokenType::LPAREN, "Expected '(' after 'print'");
    auto expr = parseExpression();
    consume(TokenType::RPAREN, "Expected ')' after print expression");
    consume(TokenType::SEMICOLON, "Expected ';' after print statement");

    return std::make_unique<PrintNode>(std::move(expr));
}

std::unique_ptr<ASTNode> Parser::parseIfStatement() {
    consume(TokenType::LPAREN, "Expected '(' after 'if'");
    auto condition = parseExpression();
    consume(TokenType::RPAREN, "Expected ')' after if condition");

    auto thenBranch = parseStatement();
    std::unique_ptr<ASTNode> elseBranch = nullptr;

    if (match(TokenType::KEYWORD_ELSE)) {
        elseBranch = parseStatement();
    }

    return std::make_unique<IfNode>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

std::unique_ptr<ASTNode> Parser::parseWhileStatement() {
    consume(TokenType::LPAREN, "Expected '(' after 'while'");
    auto condition = parseExpression();
    consume(TokenType::RPAREN, "Expected ')' after while condition");

    auto body = parseStatement();
    return std::make_unique<WhileNode>(std::move(condition), std::move(body));
}

std::unique_ptr<BlockNode> Parser::parseBlock() {
    auto block = std::make_unique<BlockNode>();

    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        block->addStatement(parseStatement());
    }

    consume(TokenType::RBRACE, "Expected '}' after block");
    return block;
}

// 1. Expression entry point: parses comparisons (lowest precedence in expressions)
std::unique_ptr<ASTNode> Parser::parseExpression() {
    return parseComparison();
}

// 2. Comparisons: ==, !=, <, <=, >, >=
std::unique_ptr<ASTNode> Parser::parseComparison() {
    auto left = parseArithmetic();

    while (match(TokenType::EQUAL_EQUAL) || match(TokenType::BANG_EQUAL) ||
           match(TokenType::LESS) || match(TokenType::LESS_EQUAL) ||
           match(TokenType::GREATER) || match(TokenType::GREATER_EQUAL)) {
        std::string op = previous().lexeme;
        auto right = parseArithmetic();
        left = std::make_unique<BinaryOpNode>(std::move(left), op, std::move(right));
    }

    return left;
}

// 3. Arithmetic: +, -
std::unique_ptr<ASTNode> Parser::parseArithmetic() {
    auto left = parseTerm();

    while (match(TokenType::PLUS) || match(TokenType::MINUS)) {
        std::string op = previous().lexeme;
        auto right = parseTerm();
        left = std::make_unique<BinaryOpNode>(std::move(left), op, std::move(right));
    }

    return left;
}

// 4. Term: *, /
std::unique_ptr<ASTNode> Parser::parseTerm() {
    auto left = parseFactor();

    while (match(TokenType::STAR) || match(TokenType::SLASH)) {
        std::string op = previous().lexeme;
        auto right = parseFactor();
        left = std::make_unique<BinaryOpNode>(std::move(left), op, std::move(right));
    }

    return left;
}

// 5. Factor: primary values & function calls
std::unique_ptr<ASTNode> Parser::parseFactor() {
    if (match(TokenType::NUMBER)) {
        long long val = std::stoll(previous().lexeme);
        return std::make_unique<NumberNode>(val);
    }

    if (match(TokenType::STRING)) {
        return std::make_unique<StringNode>(previous().lexeme);
    }

    if (match(TokenType::IDENTIFIER)) {
        std::string name = previous().lexeme;

        // Check if this identifier is followed by '(' -> Function Call
        if (match(TokenType::LPAREN)) {
            std::vector<std::unique_ptr<ASTNode>> arguments;
            if (!check(TokenType::RPAREN)) {
                do {
                    arguments.push_back(parseExpression());
                } while (match(TokenType::COMMA));
            }
            consume(TokenType::RPAREN, "Expected ')' after function arguments");
            return std::make_unique<CallNode>(name, std::move(arguments));
        }

        return std::make_unique<VariableAccessNode>(name);
    }

    if (match(TokenType::LPAREN)) {
        auto expr = parseExpression();
        consume(TokenType::RPAREN, "Expected ')' after grouped expression");
        return expr;
    }

    const Token& tok = peek();
    std::cerr << "\033[91m[Parser Error]\033[0m Unexpected token '" 
              << tok.lexeme << "' in expression at line " << tok.line << ", col " << tok.column << "\n";
    throw std::runtime_error("Unexpected token in expression");
}
