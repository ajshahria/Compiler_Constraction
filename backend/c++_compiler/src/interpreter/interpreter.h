// interpreter.h - executes TAC directly so the compiled program can be run and checked.
#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <ostream>
#include "tac.h"

// Runs the program, writing 'print' output to os. Returns false on a runtime error.
bool interpret(const TacProgram& p, std::ostream& os);

#endif
