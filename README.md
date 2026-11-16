# M-Lang Compiler

A custom programming language and compiler toolchain built from scratch in **C++17**, operating entirely in the terminal.

```text
  __  __        ____                      _ _           
 |  \/  |      / ___|___  _ __ ___  _ __ (_) | ___ _ __ 
 | |\/| |_____| |   / _ \| '_ ` _ \| '_ \| | |/ _ \ '__|
 | |  | |_____| |__| (_) | | | | | | |_) | | |  __/ |   
 |_|  |_|      \____\___/|_| |_| |_| .__/|_|_|\___|_|   
                                   |_|                  
============================================================
         Modern Custom Compiler - C++ Edition               
============================================================
```

## Features

- **Lexer (Scanner)**: Character-by-character tokenization with precise line & column tracking, whitespace handling, and comments (`//`).
- **Parser**: Handcrafted recursive descent parser enforcing operator precedence ($*, / > +, - > \text{comparisons}$).
- **Abstract Syntax Tree (AST)**: Polymorphic node hierarchy managed with `std::unique_ptr` ensuring zero memory leaks through RAII.
- **Dynamic Scoping & Environments**: Scoped symbol tables with lexical closure support (`std::shared_ptr<Environment>`).
- **Control Flow**:
  - `if` and `else` conditional branching.
  - `while` loops with state mutations (`x = x - 1;`).
- **Comparisons**: Full suite of relational operators: `==`, `!=`, `<`, `<=`, `>`, `>=`.
- **Functions & Recursion**: First-class user-defined functions (`fn`), parameters, explicit `return` statements, and recursive call frames.
- **Interactive Terminal REPL**: Interactive prompt (`./compiler repl`) for evaluating expressions live.
- **Diagnostics & Visualizers**: CLI commands to view the raw token table (`./compiler tokens`) and syntax tree (`./compiler ast`).

---

## Architecture Overview

```
Source Code (.mylang)
       │
       ▼
1. Lexical Analysis (Scanner)    --> Token stream [IDENTIFIER, EQUALS, NUMBER, ...]
       │
       ▼
2. Syntax Analysis (Parser)      --> Abstract Syntax Tree (AST) via std::unique_ptr
       │
       ▼
3. Scoped Environment            --> Symbol table with parent-pointer delegation
       │
       ▼
4. Evaluation Engine (Runtime)   --> Evaluates tree, executes loops & call frames
       │
       ▼
Terminal Output / REPL
```

---

## Building the Compiler

### Prerequisites
- Clang++ or G++ supporting C++17
- Make

### Compilation
```bash
make
```
This produces the standalone `./compiler` binary.

---

## Usage

### 1. Run a Program File
```bash
./compiler run showcase.mylang
```

### 2. Interactive Terminal REPL
Launch the interactive shell:
```bash
./compiler repl
```
```text
M-Lang >>> let a = 15;
M-Lang >>> let b = 25;
M-Lang >>> print("Sum: " + (a + b));
Sum: 40
M-Lang >>> fn square(x) { return x * x; }
M-Lang >>> print(square(8));
64
M-Lang >>> exit
```

### 3. Inspect Lexer Tokens
```bash
./compiler tokens showcase.mylang
```

### 4. Inspect the Abstract Syntax Tree (AST)
```bash
./compiler ast showcase.mylang
```

---

## Example Syntax (M-Lang)

```javascript
// Recursive Factorial
fn factorial(n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

// Iterative Range Sum
fn sumUpTo(max) {
    let total = 0;
    let i = 1;
    while (i <= max) {
        total = total + i;
        i = i + 1;
    }
    return total;
}

let fact5 = factorial(5);
print("Factorial of 5 is: " + fact5); // 120

let sum10 = sumUpTo(10);
print("Sum of 1 to 10 is: " + sum10); // 55
```

---

## Project Structure

```
.
├── Makefile                # Build configuration
├── README.md               # Documentation
├── .gitignore              # Ignores build artifacts and binaries
├── sample.mylang           # Sample test file
├── showcase.mylang         # Comprehensive feature showcase
└── src
    ├── ast.h               # AST Node definitions (std::unique_ptr)
    ├── environment.h       # Scoped symbol table (std::shared_ptr)
    ├── interpreter.h       # Tree-walking execution engine
    ├── interpreter.cpp     # Execution & call frame implementation
    ├── lexer.h             # Token definitions & Lexer header
    ├── lexer.cpp           # Scanner implementation
    ├── main.cpp            # CLI entry point, argument parsing & REPL
    ├── parser.h            # Recursive descent parser header
    ├── parser.cpp          # Grammar parsing & precedence climbing
    └── value.h             # Dynamic Value & Function object types
```

---

## License
MIT
