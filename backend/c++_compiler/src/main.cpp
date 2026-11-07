// main.cpp - compiler driver: runs the six phases in order.
//   lexer -> parser (AST) -> semantic (symbol table) -> TAC -> optimizer -> codegen / interpreter
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include "ast.h"
#include "codegen.h"
#include "diagnostics.h"
#include "headers.h"
#include "interpreter.h"
#include "optimizer.h"
#include "semantic.h"
#include "symbol_table.h"
#include "tac.h"

extern FILE* yyin;
extern int yyparse();
extern int g_dumpTokens;
extern BlockNode* g_program;

static void usage() {
    std::cout <<
        "Usage: compiler [options] <source-file>\n"
        "  --headers  list the #include headers found (preprocessor step)\n"
        "  --tokens   print the token stream (phase 1)\n"
        "  --ast      print the abstract syntax tree (phase 2)\n"
        "  --symtab   print the symbol table (phase 3)\n"
        "  --tac      print three address code (phase 4)   [default]\n"
        "  --opt      print optimized TAC (phase 5)\n"
        "  --asm      print x86 assembly (phase 6)\n"
        "  --run      execute the program with the TAC interpreter\n"
        "  --all      tokens, ast, symtab, tac, opt, asm and run\n"
        "  -o <file>  write the assembly to <file>\n";
}

int main(int argc, char** argv) {
    bool wHeaders = false, wTokens = false, wAst = false, wSym = false, wTac = false;
    bool wOpt = false, wAsm = false, wRun = false;
    const char* file = NULL;
    const char* asmOut = NULL;

    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        if (a == "--headers") wHeaders = true;
        else if (a == "--tokens") wTokens = true;
        else if (a == "--ast") wAst = true;
        else if (a == "--symtab") wSym = true;
        else if (a == "--tac") wTac = true;
        else if (a == "--opt") wOpt = true;
        else if (a == "--asm") wAsm = true;
        else if (a == "--run") wRun = true;
        else if (a == "--all") wHeaders = wTokens = wAst = wSym = wTac = wOpt = wAsm = wRun = true;
        else if (a == "-o" && i + 1 < argc) { asmOut = argv[++i]; wAsm = true; }
        else if (a == "--help" || a == "-h") { usage(); return 0; }
        else file = argv[i];
    }
    if (!file) { usage(); return 1; }
    if (!(wHeaders || wTokens || wAst || wSym || wTac || wOpt || wAsm || wRun)) wTac = true;

    yyin = fopen(file, "r");
    if (!yyin) { std::cerr << "Cannot open file '" << file << "'\n"; return 1; }

    // Phases 1 + 2: the parser pulls tokens from the lexer and builds the AST.
    g_dumpTokens = wTokens;
    if (wTokens) std::cout << "=== Tokens ===\n";
    yyparse();
    fclose(yyin);

    if (errorCount(PH_LEXICAL) + errorCount(PH_SYNTAX) > 0) {
        std::cerr << "Compilation failed: " << errorCount(PH_LEXICAL) << " lexical and "
                  << errorCount(PH_SYNTAX) << " syntax error(s).\n";
        return 1;
    }
    if (wHeaders) { std::cout << "=== Headers ===\n"; printHeaders(std::cout); }
    if (wAst) { std::cout << "=== AST ===\n"; g_program->print(std::cout, 0); }

    // Phase 3: semantic analysis with the symbol table.
    SymbolTable table;
    int semErrors = analyze(g_program, table);
    if (wSym) { std::cout << "=== Symbol Table ===\n"; table.dump(std::cout); }
    if (semErrors > 0) {
        std::cerr << "Compilation failed: " << semErrors << " semantic error(s).\n";
        return 1;
    }

    // Phase 4: intermediate code.
    TacProgram tac = generateTAC(g_program, table);
    if (wTac) { std::cout << "=== Three Address Code ===\n"; printTAC(tac, std::cout); }

    // Phase 5: optimization.
    TacProgram opt = optimize(tac);
    if (wOpt) { std::cout << "=== Optimized TAC ===\n"; printTAC(opt, std::cout); }

    // Phase 6: code generation (+ interpreter to run the result).
    if (wAsm) {
        std::string text = generateAssembly(opt);
        if (asmOut) {
            std::ofstream f(asmOut);
            f << text;
            std::cout << "Assembly written to " << asmOut << "\n";
        } else {
            std::cout << "=== x86 Assembly ===\n" << text;
        }
    }
    if (wRun) {
        std::cout << "=== Program Output ===\n";
        if (!interpret(opt, std::cout)) return 2;
    }
    return 0;
}
