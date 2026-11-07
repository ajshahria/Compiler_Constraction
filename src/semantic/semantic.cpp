// semantic.cpp - rules the grammar cannot express:
//   undeclared variable, redeclaration, scope violation, type mismatch,
//   invalid assignment, invalid operands, non-bool conditions.
// int -> float widening is allowed; every other mixing is an error.
#include "semantic.h"
#include "diagnostics.h"
#include "headers.h"
#include <sstream>

static SymbolTable* table;
static int errors;

static void err(int line, const std::string& msg) {
    errors++;
    reportError(PH_SEMANTIC, line, msg);
}

static bool canAssign(Type to, Type from) {
    if (to == from) return true;
    return to == TY_FLOAT && from == TY_INT;   // widening only
}

static Type checkExpr(Node* n);
static void checkStmt(Node* n);

static Type checkBinary(BinaryNode* b) {
    Type l = checkExpr(b->left);
    Type r = checkExpr(b->right);
    if (l == TY_ERROR || r == TY_ERROR) return TY_ERROR;   // already reported, avoid cascades
    const std::string& op = b->op;

    if (op == "+" || op == "-" || op == "*" || op == "/") {
        if (!isNumeric(l) || !isNumeric(r)) {
            err(b->line, "invalid operands to '" + op + "': " + typeName(l) + " and " +
                typeName(r) + " (arithmetic needs int or float)");
            return TY_ERROR;
        }
        return (l == TY_FLOAT || r == TY_FLOAT) ? TY_FLOAT : TY_INT;
    }
    if (op == "%") {
        if (l != TY_INT || r != TY_INT) {
            err(b->line, std::string("invalid operands to '%': ") + typeName(l) + " and " +
                typeName(r) + " ('%' needs two int operands)");
            return TY_ERROR;
        }
        return TY_INT;
    }
    if (op == "<" || op == ">" || op == "<=" || op == ">=") {
        if (!isNumeric(l) || !isNumeric(r)) {
            err(b->line, "invalid operands to '" + op + "': " + typeName(l) + " and " +
                typeName(r) + " (relational needs int or float)");
            return TY_ERROR;
        }
        return TY_BOOL;
    }
    if (op == "==" || op == "!=") {
        bool ok = (isNumeric(l) && isNumeric(r)) || (l == TY_BOOL && r == TY_BOOL);
        if (!ok) {
            err(b->line, "invalid operands to '" + op + "': cannot compare " + typeName(l) +
                " with " + typeName(r));
            return TY_ERROR;
        }
        return TY_BOOL;
    }
    // && and ||
    if (l != TY_BOOL || r != TY_BOOL) {
        err(b->line, "invalid operands to '" + op + "': " + typeName(l) + " and " +
            typeName(r) + " (logical operators need bool operands)");
        return TY_ERROR;
    }
    return TY_BOOL;
}

static Type checkExpr(Node* n) {
    Type t = TY_ERROR;
    switch (n->kind) {
        case NodeKind::IntLit:   t = TY_INT;   break;
        case NodeKind::FloatLit: t = TY_FLOAT; break;
        case NodeKind::BoolLit:  t = TY_BOOL;  break;
        case NodeKind::Ident: {
            IdentNode* id = static_cast<IdentNode*>(n);
            Symbol* s = table->lookup(id->name);
            if (s) {
                id->uname = s->uname;
                t = s->type;
            } else if (Symbol* c = table->findClosed(id->name)) {
                std::ostringstream m;
                m << "scope violation: '" << id->name << "' was declared in an inner block (line "
                  << c->line << ") and is not visible here";
                err(id->line, m.str());
            } else {
                err(id->line, "undeclared variable '" + id->name + "'");
            }
            break;
        }
        case NodeKind::Unary: {
            UnaryNode* u = static_cast<UnaryNode*>(n);
            Type o = checkExpr(u->operand);
            if (o == TY_ERROR) break;
            if (u->op == "!") {
                if (o != TY_BOOL) err(u->line, std::string("invalid operand to '!': ") + typeName(o) + " (needs bool)");
                else t = TY_BOOL;
            } else {
                if (!isNumeric(o)) err(u->line, std::string("invalid operand to unary '-': ") + typeName(o) + " (needs int or float)");
                else t = o;
            }
            break;
        }
        case NodeKind::Binary:
            t = checkBinary(static_cast<BinaryNode*>(n));
            break;
        default:
            break;
    }
    n->type = t;
    return t;
}

static void checkCondition(Node* cond, const char* what) {
    Type t = checkExpr(cond);
    if (t != TY_ERROR && t != TY_BOOL)
        err(cond->line, std::string("condition of '") + what + "' must be bool, found " + typeName(t));
}

static void checkBlockBody(BlockNode* b) {
    for (size_t i = 0; i < b->stmts.size(); i++) checkStmt(b->stmts[i]);
}

static void checkStmt(Node* n) {
    switch (n->kind) {
        case NodeKind::Block:
            table->enterScope();
            checkBlockBody(static_cast<BlockNode*>(n));
            table->exitScope();
            break;
        case NodeKind::VarDecl: {
            VarDeclNode* d = static_cast<VarDeclNode*>(n);
            // Check the initializer first: 'int x = x;' must not see the new x.
            Type it = d->init ? checkExpr(d->init) : TY_ERROR;
            Symbol *existing = NULL, *created = NULL;
            if (!table->insert(d->name, d->declType, d->line, &existing, &created)) {
                std::ostringstream m;
                m << "redeclaration of '" << d->name << "' in the same scope (first declared at line "
                  << existing->line << ")";
                err(d->line, m.str());
            } else {
                d->uname = created->uname;
            }
            if (d->init && it != TY_ERROR && !canAssign(d->declType, it))
                err(d->line, std::string("type mismatch: cannot initialize ") + typeName(d->declType) +
                    " '" + d->name + "' with a value of type " + typeName(it));
            break;
        }
        case NodeKind::Assign: {
            AssignNode* a = static_cast<AssignNode*>(n);
            Type vt = checkExpr(a->value);
            Symbol* s = table->lookup(a->name);
            if (!s) {
                if (Symbol* c = table->findClosed(a->name)) {
                    std::ostringstream m;
                    m << "scope violation: '" << a->name << "' was declared in an inner block (line "
                      << c->line << ") and is not visible here";
                    err(a->line, m.str());
                } else {
                    err(a->line, "undeclared variable '" + a->name + "'");
                }
                break;
            }
            a->uname = s->uname;
            a->type = s->type;
            if (vt != TY_ERROR && !canAssign(s->type, vt))
                err(a->line, std::string("invalid assignment: cannot assign ") + typeName(vt) +
                    " to " + typeName(s->type) + " variable '" + a->name + "'");
            break;
        }
        case NodeKind::If: {
            IfNode* f = static_cast<IfNode*>(n);
            checkCondition(f->cond, "if");
            checkStmt(f->thenStmt);   // a single non-block statement still gets checked
            if (f->elseStmt) checkStmt(f->elseStmt);
            break;
        }
        case NodeKind::While: {
            WhileNode* w = static_cast<WhileNode*>(n);
            checkCondition(w->cond, "while");
            checkStmt(w->body);
            break;
        }
        case NodeKind::Print: {
            PrintNode* p = static_cast<PrintNode*>(n);
            if (p->viaCout && !hasHeader("iostream"))
                err(p->line, "'cout' is not declared: add #include <iostream>");
            checkExpr(p->expr);
            break;
        }
        default:
            break;
    }
}

int analyze(BlockNode* program, SymbolTable& symtab) {
    table = &symtab;
    errors = 0;
    checkBlockBody(program);   // the program itself is the global scope (already open)
    return errors;
}
