/* parser.y - Phase 2: Syntax analysis (Bison). Builds the AST.
 *
 * Grammar (see docs/grammar.md):
 *   program   -> stmt_list
 *   stmt      -> decl | assign ; | if | while | for | print | block | ;
 * Operator precedence (low -> high): ||  &&  == !=  < > <= >=  + -  * / %  ! unary-
 * Dangling else is resolved with %prec: else binds to the nearest if.
 * Error recovery: 'error ;' skips to the end of a bad statement,
 * 'error }' skips to the end of a bad block.
 *
 * C++ extras (bonus) are rewritten into core nodes right here in the parser:
 *   x += e  ->  x = x + e        x++  ->  x = x + 1        cout << e  ->  print e
 *   for (i; c; s) b  ->  { i; while (c) { b; s; } }
 * so semantic analysis, TAC and code generation need no changes.
 */
%{
#include <cstdio>
#include <cstdlib>
#include <string>
#include "ast.h"
#include "diagnostics.h"

extern int yylex();
extern int yylineno;
extern int g_lastLexErrorLine;
void yyerror(const char* msg);

BlockNode* g_program = NULL;   /* root of the AST, read by main */

/* build  name = name <op> rhs  (used by +=, -=, ++ ...) */
static Node* compound(const char* name, const char* op, Node* rhs, int line) {
    return new AssignNode(name, new BinaryNode(op, new IdentNode(name, line), rhs, line), line);
}
%}

%error-verbose
%locations

%code requires {
#include "ast.h"
}

%union {
    long        ival;
    double      fval;
    char*       sval;
    Type        tval;
    Node*       node;
    BlockNode*  block;
}

%token KW_INT KW_FLOAT KW_BOOL KW_IF KW_ELSE KW_WHILE KW_PRINT KW_TRUE KW_FALSE
/* C++ extras (bonus) */
%token KW_FOR KW_COUT SHL INC DEC ADDEQ SUBEQ MULEQ DIVEQ
%token <ival> INT_LIT
%token <fval> FLOAT_LIT
%token <sval> IDENTIFIER
%token LE GE EQ NE AND OR

%type <tval>  type
%type <node>  stmt decl assign_stmt simple_assign if_stmt while_stmt for_stmt print_stmt expr block
%type <node>  for_init for_cond for_step
%type <block> stmt_list program

%nonassoc LOWER_THAN_ELSE
%nonassoc KW_ELSE
%left OR
%left AND
%left EQ NE
%left '<' '>' LE GE
%left '+' '-'
%left '*' '/' '%'
%right '!' UMINUS

%%

program
    : stmt_list                 { g_program = $1; $$ = $1; }
    ;

stmt_list
    : /* empty */               { $$ = new BlockNode(NodeKind::Program, yylineno); }
    | stmt_list stmt            { if ($2) $1->stmts.push_back($2); $$ = $1; }
    ;

stmt
    : decl
    | assign_stmt
    | if_stmt
    | while_stmt
    | for_stmt
    | print_stmt
    | block
    | ';'                       { $$ = NULL; }
    | error ';'                 { yyerrok; $$ = NULL; }
    ;

block
    : '{' stmt_list '}'         { $2->kind = NodeKind::Block; $$ = $2; }
    | '{' stmt_list error '}'   { yyerrok; $2->kind = NodeKind::Block; $$ = $2; }
    ;

type
    : KW_INT                    { $$ = TY_INT; }
    | KW_FLOAT                  { $$ = TY_FLOAT; }
    | KW_BOOL                   { $$ = TY_BOOL; }
    ;

decl
    : type IDENTIFIER ';'           { $$ = new VarDeclNode($1, $2, NULL, @1.first_line); free($2); }
    | type IDENTIFIER '=' expr ';'  { $$ = new VarDeclNode($1, $2, $4, @1.first_line); free($2); }
    ;

assign_stmt
    : simple_assign ';'         { $$ = $1; }
    ;

/* an assignment without its ';' (also used inside for(...)) */
simple_assign
    : IDENTIFIER '=' expr       { $$ = new AssignNode($1, $3, @1.first_line); free($1); }
    | IDENTIFIER ADDEQ expr     { $$ = compound($1, "+", $3, @1.first_line); free($1); }
    | IDENTIFIER SUBEQ expr     { $$ = compound($1, "-", $3, @1.first_line); free($1); }
    | IDENTIFIER MULEQ expr     { $$ = compound($1, "*", $3, @1.first_line); free($1); }
    | IDENTIFIER DIVEQ expr     { $$ = compound($1, "/", $3, @1.first_line); free($1); }
    | IDENTIFIER INC            { $$ = compound($1, "+", new IntLitNode(1, @1.first_line), @1.first_line); free($1); }
    | IDENTIFIER DEC            { $$ = compound($1, "-", new IntLitNode(1, @1.first_line), @1.first_line); free($1); }
    ;

if_stmt
    : KW_IF '(' expr ')' stmt %prec LOWER_THAN_ELSE
                                { $$ = new IfNode($3, $5, NULL, @1.first_line); }
    | KW_IF '(' expr ')' stmt KW_ELSE stmt
                                { $$ = new IfNode($3, $5, $7, @1.first_line); }
    ;

while_stmt
    : KW_WHILE '(' expr ')' stmt  { $$ = new WhileNode($3, $5, @1.first_line); }
    ;

for_stmt
    : KW_FOR '(' for_init ';' for_cond ';' for_step ')' stmt
        {
            BlockNode* outer = new BlockNode(NodeKind::Block, @1.first_line);
            BlockNode* inner = new BlockNode(NodeKind::Block, @1.first_line);
            if ($3) outer->stmts.push_back($3);
            if ($9) inner->stmts.push_back($9);
            if ($7) inner->stmts.push_back($7);
            outer->stmts.push_back(new WhileNode($5, inner, @1.first_line));
            $$ = outer;
        }
    ;

for_init
    : /* empty */                   { $$ = NULL; }
    | simple_assign                 { $$ = $1; }
    | type IDENTIFIER '=' expr      { $$ = new VarDeclNode($1, $2, $4, @1.first_line); free($2); }
    ;

for_cond
    : /* empty */                   { $$ = new BoolLitNode(true, yylineno); }
    | expr                          { $$ = $1; }
    ;

for_step
    : /* empty */                   { $$ = NULL; }
    | simple_assign                 { $$ = $1; }
    ;

print_stmt
    : KW_PRINT expr ';'         { $$ = new PrintNode($2, @1.first_line); }
    | KW_COUT SHL expr ';'      { $$ = new PrintNode($3, @1.first_line); }
    ;

expr
    : expr OR expr              { $$ = new BinaryNode("||", $1, $3, @2.first_line); }
    | expr AND expr             { $$ = new BinaryNode("&&", $1, $3, @2.first_line); }
    | expr EQ expr              { $$ = new BinaryNode("==", $1, $3, @2.first_line); }
    | expr NE expr              { $$ = new BinaryNode("!=", $1, $3, @2.first_line); }
    | expr '<' expr             { $$ = new BinaryNode("<", $1, $3, @2.first_line); }
    | expr '>' expr             { $$ = new BinaryNode(">", $1, $3, @2.first_line); }
    | expr LE expr              { $$ = new BinaryNode("<=", $1, $3, @2.first_line); }
    | expr GE expr              { $$ = new BinaryNode(">=", $1, $3, @2.first_line); }
    | expr '+' expr             { $$ = new BinaryNode("+", $1, $3, @2.first_line); }
    | expr '-' expr             { $$ = new BinaryNode("-", $1, $3, @2.first_line); }
    | expr '*' expr             { $$ = new BinaryNode("*", $1, $3, @2.first_line); }
    | expr '/' expr             { $$ = new BinaryNode("/", $1, $3, @2.first_line); }
    | expr '%' expr             { $$ = new BinaryNode("%", $1, $3, @2.first_line); }
    | '!' expr                  { $$ = new UnaryNode("!", $2, @1.first_line); }
    | '-' expr %prec UMINUS     { $$ = new UnaryNode("-", $2, @1.first_line); }
    | '(' expr ')'              { $$ = $2; }
    | INT_LIT                   { $$ = new IntLitNode($1, @1.first_line); }
    | FLOAT_LIT                 { $$ = new FloatLitNode($1, @1.first_line); }
    | KW_TRUE                   { $$ = new BoolLitNode(true, @1.first_line); }
    | KW_FALSE                  { $$ = new BoolLitNode(false, @1.first_line); }
    | IDENTIFIER                { $$ = new IdentNode($1, @1.first_line); free($1); }
    ;

%%

void yyerror(const char* msg) {
    /* a bad token on this line was already reported by the lexer: don't pile on */
    if (g_lastLexErrorLine == yylineno) return;
    reportError(PH_SYNTAX, yylineno, msg);
}
