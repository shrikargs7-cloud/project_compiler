#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>

// Enumeration of all token types our language supports
enum class TokenType {
    // Keywords
    KEYWORD_LET,
    KEYWORD_PRINT,
    KEYWORD_IF,
    KEYWORD_ELSE,
    KEYWORD_WHILE,
    KEYWORD_FN,
    KEYWORD_RETURN,

    // Literals & Identifiers
    IDENTIFIER,
    NUMBER,
    STRING,

    // Arithmetic Operators
    PLUS,
    MINUS,
    STAR,
    SLASH,

    // Assignment & Comparison Operators
    EQUALS,         // =
    EQUAL_EQUAL,    // ==
    BANG_EQUAL,     // !=
    LESS,           // <
    LESS_EQUAL,     // <=
    GREATER,        // >
    GREATER_EQUAL,  // >=

    // Punctuation
    LPAREN,     // (
    RPAREN,     // )
    LBRACE,     // {
    RBRACE,     // }
    COMMA,      // ,
    SEMICOLON,  // ;

    // Special
    END_OF_FILE,
    UNKNOWN
};

// Represents a single token produced by lexical analysis
struct Token {
    TokenType type;
    std::string lexeme; // The actual string slice from source code
    size_t line;        // Line number
    size_t column;      // Column number
};

class Lexer {
private:
    std::string source;
    size_t cursor;      // Current character index in source string
    size_t line;        // Current line number
    size_t column;      // Current column number

    // Helper methods
    char peek() const;
    char advance();
    bool isAtEnd() const;
    void skipWhitespaceAndComments();
    Token scanNumber();
    Token scanIdentifierOrKeyword();
    Token scanString();

public:
    explicit Lexer(std::string source_code);
    std::vector<Token> tokenize();
    static std::string tokenTypeToString(TokenType type);
};

#endif
