#include "lexer.h"
#include <iostream>
#include <cctype>

Lexer::Lexer(std::string source_code)
    : source(std::move(source_code)), cursor(0), line(1), column(1) {}

bool Lexer::isAtEnd() const {
    return cursor >= source.size();
}

char Lexer::peek() const {
    if (isAtEnd()) return '\0';
    return source[cursor];
}

char Lexer::advance() {
    if (isAtEnd()) return '\0';
    char current = source[cursor++];
    column++;
    return current;
}

void Lexer::skipWhitespaceAndComments() {
    while (!isAtEnd()) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r') {
            advance();
        } else if (c == '\n') {
            line++;
            column = 1;
            cursor++;
        } else if (c == '/' && cursor + 1 < source.size() && source[cursor + 1] == '/') {
            while (!isAtEnd() && peek() != '\n') {
                advance();
            }
        } else {
            break;
        }
    }
}

Token Lexer::scanNumber() {
    size_t startCol = column;
    std::string numStr;

    while (!isAtEnd() && std::isdigit(peek())) {
        numStr += advance();
    }

    return {TokenType::NUMBER, numStr, line, startCol};
}

Token Lexer::scanIdentifierOrKeyword() {
    size_t startCol = column;
    std::string ident;

    while (!isAtEnd() && (std::isalnum(peek()) || peek() == '_')) {
        ident += advance();
    }

    TokenType type = TokenType::IDENTIFIER;
    if (ident == "let")         type = TokenType::KEYWORD_LET;
    else if (ident == "print")  type = TokenType::KEYWORD_PRINT;
    else if (ident == "if")     type = TokenType::KEYWORD_IF;
    else if (ident == "else")   type = TokenType::KEYWORD_ELSE;
    else if (ident == "while")  type = TokenType::KEYWORD_WHILE;
    else if (ident == "fn")     type = TokenType::KEYWORD_FN;
    else if (ident == "return") type = TokenType::KEYWORD_RETURN;

    return {type, ident, line, startCol};
}

Token Lexer::scanString() {
    size_t startCol = column;
    advance(); // Consume opening quote '"'
    std::string strVal;

    while (!isAtEnd() && peek() != '"') {
        if (peek() == '\n') {
            line++;
            column = 1;
        }
        strVal += advance();
    }

    if (isAtEnd()) {
        std::cerr << "\033[91m[Lexer Error]\033[0m Unterminated string literal at line " 
                  << line << ", col " << startCol << "\n";
        return {TokenType::UNKNOWN, strVal, line, startCol};
    }

    advance(); // Consume closing quote '"'
    return {TokenType::STRING, strVal, line, startCol};
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (true) {
        skipWhitespaceAndComments();

        if (isAtEnd()) {
            tokens.push_back({TokenType::END_OF_FILE, "", line, column});
            break;
        }

        size_t startCol = column;
        char c = peek();

        if (std::isdigit(c)) {
            tokens.push_back(scanNumber());
        } else if (std::isalpha(c) || c == '_') {
            tokens.push_back(scanIdentifierOrKeyword());
        } else if (c == '"') {
            tokens.push_back(scanString());
        } else {
            advance();
            switch (c) {
                case '+': tokens.push_back({TokenType::PLUS, "+", line, startCol}); break;
                case '-': tokens.push_back({TokenType::MINUS, "-", line, startCol}); break;
                case '*': tokens.push_back({TokenType::STAR, "*", line, startCol}); break;
                case '/': tokens.push_back({TokenType::SLASH, "/", line, startCol}); break;
                case '(': tokens.push_back({TokenType::LPAREN, "(", line, startCol}); break;
                case ')': tokens.push_back({TokenType::RPAREN, ")", line, startCol}); break;
                case '{': tokens.push_back({TokenType::LBRACE, "{", line, startCol}); break;
                case '}': tokens.push_back({TokenType::RBRACE, "}", line, startCol}); break;
                case ',': tokens.push_back({TokenType::COMMA, ",", line, startCol}); break;
                case ';': tokens.push_back({TokenType::SEMICOLON, ";", line, startCol}); break;

                case '=':
                    if (peek() == '=') {
                        advance();
                        tokens.push_back({TokenType::EQUAL_EQUAL, "==", line, startCol});
                    } else {
                        tokens.push_back({TokenType::EQUALS, "=", line, startCol});
                    }
                    break;

                case '!':
                    if (peek() == '=') {
                        advance();
                        tokens.push_back({TokenType::BANG_EQUAL, "!=", line, startCol});
                    } else {
                        tokens.push_back({TokenType::UNKNOWN, "!", line, startCol});
                    }
                    break;

                case '<':
                    if (peek() == '=') {
                        advance();
                        tokens.push_back({TokenType::LESS_EQUAL, "<=", line, startCol});
                    } else {
                        tokens.push_back({TokenType::LESS, "<", line, startCol});
                    }
                    break;

                case '>':
                    if (peek() == '=') {
                        advance();
                        tokens.push_back({TokenType::GREATER_EQUAL, ">=", line, startCol});
                    } else {
                        tokens.push_back({TokenType::GREATER, ">", line, startCol});
                    }
                    break;

                default:
                    std::cerr << "\033[91m[Lexer Error]\033[0m Unexpected character '" 
                              << c << "' at line " << line << ", col " << startCol << "\n";
                    tokens.push_back({TokenType::UNKNOWN, std::string(1, c), line, startCol});
                    break;
            }
        }
    }

    return tokens;
}

std::string Lexer::tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::KEYWORD_LET:     return "KEYWORD_LET";
        case TokenType::KEYWORD_PRINT:   return "KEYWORD_PRINT";
        case TokenType::KEYWORD_IF:      return "KEYWORD_IF";
        case TokenType::KEYWORD_ELSE:    return "KEYWORD_ELSE";
        case TokenType::KEYWORD_WHILE:   return "KEYWORD_WHILE";
        case TokenType::KEYWORD_FN:      return "KEYWORD_FN";
        case TokenType::KEYWORD_RETURN:  return "KEYWORD_RETURN";
        case TokenType::IDENTIFIER:      return "IDENTIFIER";
        case TokenType::NUMBER:          return "NUMBER";
        case TokenType::STRING:          return "STRING";
        case TokenType::PLUS:            return "PLUS";
        case TokenType::MINUS:           return "MINUS";
        case TokenType::STAR:            return "STAR";
        case TokenType::SLASH:           return "SLASH";
        case TokenType::EQUALS:          return "EQUALS";
        case TokenType::EQUAL_EQUAL:     return "EQUAL_EQUAL";
        case TokenType::BANG_EQUAL:      return "BANG_EQUAL";
        case TokenType::LESS:            return "LESS";
        case TokenType::LESS_EQUAL:      return "LESS_EQUAL";
        case TokenType::GREATER:         return "GREATER";
        case TokenType::GREATER_EQUAL:   return "GREATER_EQUAL";
        case TokenType::LPAREN:          return "LPAREN";
        case TokenType::RPAREN:          return "RPAREN";
        case TokenType::LBRACE:          return "LBRACE";
        case TokenType::RBRACE:          return "RBRACE";
        case TokenType::COMMA:           return "COMMA";
        case TokenType::SEMICOLON:       return "SEMICOLON";
        case TokenType::END_OF_FILE:     return "EOF";
        case TokenType::UNKNOWN:         return "UNKNOWN";
    }
    return "UNDEFINED";
}
