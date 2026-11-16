#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include <algorithm>
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"

void show_banner() {
    std::cout << "\033[96m" << R"(
  __  __        ____                      _ _           
 |  \/  |      / ___|___  _ __ ___  _ __ (_) | ___ _ __ 
 | |\/| |_____| |   / _ \| '_ ` _ \| '_ \| | |/ _ \ '__|
 | |  | |_____| |__| (_) | | | | | | |_) | | |  __/ |   
 |_|  |_|      \____\___/|_| |_| |_| .__/|_|_|\___|_|   
                                   |_|                  
)" << "\033[0m\n";
    std::cout << "============================================================\n";
    std::cout << "         Modern Custom Compiler - C++ Edition               \n";
    std::cout << "============================================================\n\n";
}

void runRepl() {
    show_banner();
    std::cout << "Interactive M-Lang REPL. Type expressions, statements, or 'exit' to quit.\n\n";

    Interpreter interpreter;
    std::string line;

    while (true) {
        std::cout << "\033[92mM-Lang >>> \033[0m";
        if (!std::getline(std::cin, line)) {
            std::cout << "\n";
            break;
        }

        // Trim leading and trailing whitespace
        std::string trimmed = line;
        trimmed.erase(0, trimmed.find_first_not_of(" \t\r\n"));
        trimmed.erase(trimmed.find_last_not_of(" \t\r\n") + 1);

        if (trimmed == "exit" || trimmed == "quit" || trimmed == "exit;" || trimmed == "quit;") {
            std::cout << "Goodbye!\n";
            break;
        }

        if (trimmed.empty()) continue;

        try {
            Lexer lexer(trimmed);
            auto tokens = lexer.tokenize();
            Parser parser(std::move(tokens));
            auto ast = parser.parse();

            for (const auto& stmt : ast->statements) {
                interpreter.executeStatement(stmt.get());
            }
        } catch (const std::exception& e) {
            std::cerr << "\033[91m[Error]\033[0m " << e.what() << "\n";
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        show_banner();
        std::cout << "Usage: ./compiler <command> [file]\n";
        std::cout << "Commands:\n";
        std::cout << "  run <file>      Tokenize, parse, and execute a source file\n";
        std::cout << "  repl            Start interactive terminal REPL shell\n";
        std::cout << "  ast <file>      Visualize the Abstract Syntax Tree (AST)\n";
        std::cout << "  tokens <file>   Display raw tokens produced by the lexer\n";
        std::cout << "  version         Display compiler version\n";
        return 1;
    }

    std::string command = argv[1];

    if (command == "version") {
        show_banner();
        std::cout << "M-Compiler version 1.0.0 (C++ Core Architecture)\n";
        return 0;
    }

    if (command == "repl") {
        runRepl();
        return 0;
    }

    if (command == "run" || command == "tokens" || command == "ast" || command == "build") {
        if (argc < 3) {
            std::cerr << "\033[91mError: Please provide a source file (e.g., ./compiler " 
                      << command << " sample.mylang)\033[0m\n";
            return 1;
        }

        std::string filename = argv[2];
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "\033[91mError: Could not open file '" << filename << "'\033[0m\n";
            return 1;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string source_code = buffer.str();
        file.close();

        show_banner();
        std::cout << "[*] Source File: " << filename << " (" << source_code.size() << " bytes)\n";

        // ================= STAGE 1: LEXICAL ANALYSIS =================
        std::cout << "\n\033[93m[*] Stage 1: Running Lexical Analysis (Scanner)...\033[0m\n";
        Lexer lexer(source_code);
        std::vector<Token> tokens = lexer.tokenize();
        std::cout << "[+] Scanned " << tokens.size() << " tokens successfully.\n";

        if (command == "tokens") {
            std::cout << "\n\033[90m" << std::left 
                      << std::setw(6)  << "Line" 
                      << std::setw(6)  << "Col" 
                      << std::setw(18) << "Token Type" 
                      << "Lexeme" 
                      << "\033[0m\n";
            std::cout << std::string(45, '-') << "\n";
            for (const auto& tok : tokens) {
                std::cout << std::left 
                          << std::setw(6)  << tok.line 
                          << std::setw(6)  << tok.column 
                          << "\033[32m" << std::setw(18) << Lexer::tokenTypeToString(tok.type) << "\033[0m"
                          << "'" << tok.lexeme << "'\n";
            }
            std::cout << std::string(45, '-') << "\n";
            return 0;
        }

        // ================= STAGE 2: PARSING (AST GENERATION) =================
        std::cout << "\n\033[93m[*] Stage 2: Syntax Analysis (Parser & AST)...\033[0m\n";
        Parser parser(tokens);
        std::unique_ptr<ProgramNode> ast = parser.parse();
        std::cout << "[+] AST successfully constructed with " 
                  << ast->statements.size() << " top-level statements.\n";

        if (command == "ast") {
            std::cout << "\n";
            ast->print();
            return 0;
        }

        // ================= STAGE 3: EVALUATION / EXECUTION =================
        std::cout << "\n\033[93m[*] Stage 3: Evaluation Engine (Executing)...\033[0m\n";
        std::cout << "\n\033[92m--- PROGRAM OUTPUT ---\033[0m\n";
        Interpreter interpreter;
        interpreter.interpret(ast.get());
        std::cout << "\033[92m----------------------\033[0m\n\n";
        std::cout << "[+] Program finished execution with code 0.\n";

    } else {
        std::cerr << "Unknown command: " << command << "\n";
        return 1;
    }

    return 0;
}
