# Formal Grammar (CFG)

Terminals are in `monospace`; `ε` is the empty string. The grammar is implemented in
[`src/parser/parser.y`](../src/parser/parser.y).

## Core language (manual §5)

```
program      → stmt_list
stmt_list    → ε | stmt_list stmt
stmt         → decl | assign_stmt | if_stmt | while_stmt | print_stmt | block | ';'
block        → '{' stmt_list '}'
type         → 'int' | 'float' | 'bool'
decl         → type ID ';'  |  type ID '=' expr ';'
assign_stmt  → ID '=' expr ';'
if_stmt      → 'if' '(' expr ')' stmt
             | 'if' '(' expr ')' stmt 'else' stmt
while_stmt   → 'while' '(' expr ')' stmt
print_stmt   → 'print' expr ';'

expr         → expr '||' expr | expr '&&' expr
             | expr '==' expr | expr '!=' expr
             | expr '<' expr  | expr '>' expr | expr '<=' expr | expr '>=' expr
             | expr '+' expr  | expr '-' expr
             | expr '*' expr  | expr '/' expr | expr '%' expr
             | '!' expr | '-' expr | '(' expr ')'
             | INT_LIT | FLOAT_LIT | 'true' | 'false' | ID
```

The `expr` rule is ambiguous as written; Bison resolves it with precedence declarations.

| Level (low → high) | Operators | Associativity |
|---|---|---|
| 1 | `\|\|` | left |
| 2 | `&&` | left |
| 3 | `==` `!=` | left |
| 4 | `<` `>` `<=` `>=` | left |
| 5 | `+` `-` | left |
| 6 | `*` `/` `%` | left |
| 7 | `!` unary `-` | right |

**Dangling else:** `if (a) if (b) x; else y;` – the `else` belongs to the nearest `if`
(rule `if` without else has the lower precedence `LOWER_THAN_ELSE`, so Bison shifts `else`).

## Lexical elements

```
ID        → [A-Za-z_][A-Za-z0-9_]*
INT_LIT   → [0-9]+
FLOAT_LIT → [0-9]+ '.' [0-9]+
comments  → '//' … end of line   |   '/*' … '*/'     (discarded)
```

## C++ extras (bonus)

Rewritten into core nodes by the parser:

```
simple_assign → ID '=' expr | ID '+=' expr | ID '-=' expr | ID '*=' expr | ID '/=' expr
              | ID '++' | ID '--'
assign_stmt   → simple_assign ';'
print_stmt    → 'cout' '<<' expr ';'
for_stmt      → 'for' '(' for_init ';' for_cond ';' for_step ')' stmt
for_init      → ε | simple_assign | type ID '=' expr
for_cond      → ε | expr                 (ε means true)
for_step      → ε | simple_assign
```

## Error recovery

`stmt → error ';'` skips to the end of a broken statement and `block → '{' stmt_list error '}'`
skips to the end of a broken block, so several syntax errors can be reported in one run.
