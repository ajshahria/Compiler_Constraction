/* lexer.l - Phase 1: Lexical analysis (Flex).
 * Turns characters into tokens, discards comments/whitespace,
 * reports invalid tokens with the line number.
 */
%{
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include "ast.h"
#include "diagnostics.h"
#include "parser.tab.h"

int g_dumpTokens = 0;
int g_lastLexErrorLine = 0;   /* used by the parser to suppress follow-up syntax errors */
static int commentStart = 0;   /* set by main (--tokens) to print every token */

/* Print the token when --tokens is on, then hand it to the parser. */
static int tok(int t, const char* name) {
    if (g_dumpTokens)
        printf("%-12s %-10s line %d\n", name, yytext, yylineno);
    return t;
}
#define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno;
#define TOK(t) return tok(t, #t)
%}

%option noyywrap
%option nounput
%option yylineno

DIGIT   [0-9]
LETTER  [A-Za-z_]

%x BLOCKCOMMENT

%%

"int"       { TOK(KW_INT); }
"float"     { TOK(KW_FLOAT); }
"bool"      { TOK(KW_BOOL); }
"if"        { TOK(KW_IF); }
"else"      { TOK(KW_ELSE); }
"while"     { TOK(KW_WHILE); }
"print"     { TOK(KW_PRINT); }
"true"      { TOK(KW_TRUE); }
"false"     { TOK(KW_FALSE); }
"for"       { TOK(KW_FOR); }
"cout"      { TOK(KW_COUT); }

{DIGIT}+"."{DIGIT}+  { yylval.fval = atof(yytext); TOK(FLOAT_LIT); }
{DIGIT}+             { yylval.ival = atol(yytext); TOK(INT_LIT); }

{DIGIT}+{LETTER}({LETTER}|{DIGIT})* {
    g_lastLexErrorLine = yylineno;
    reportError(PH_LEXICAL, yylineno,
                std::string("invalid identifier '") + yytext + "' (cannot start with a digit)");
}

{LETTER}({LETTER}|{DIGIT})* {
    yylval.sval = strdup(yytext);
    TOK(IDENTIFIER);
}

"<<"    { TOK(SHL); }
"++"    { TOK(INC); }
"--"    { TOK(DEC); }
"+="    { TOK(ADDEQ); }
"-="    { TOK(SUBEQ); }
"*="    { TOK(MULEQ); }
"/="    { TOK(DIVEQ); }
"<="    { TOK(LE); }
">="    { TOK(GE); }
"=="    { TOK(EQ); }
"!="    { TOK(NE); }
"&&"    { TOK(AND); }
"||"    { TOK(OR); }

[+\-*/%<>=!(){};]  { return tok(yytext[0], yytext); }

"//".*              { /* single-line comment: discarded */ }
"/*"                { commentStart = yylineno; BEGIN(BLOCKCOMMENT); }
<BLOCKCOMMENT>"*/"  { BEGIN(INITIAL); }
<BLOCKCOMMENT>\n    { /* yylineno is updated by %option yylineno */ }
<BLOCKCOMMENT>.     { }
<BLOCKCOMMENT><<EOF>> {
    reportError(PH_LEXICAL, commentStart, "unterminated block comment (started here)");
    BEGIN(INITIAL);
    return 0;
}

[ \t\r\n]+          { /* whitespace: discarded */ }

.   {
    g_lastLexErrorLine = yylineno;
    reportError(PH_LEXICAL, yylineno,
                std::string("invalid character '") + yytext + "'");
}

%%
