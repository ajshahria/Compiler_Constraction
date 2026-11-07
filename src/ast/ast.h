// ast.h - Abstract Syntax Tree node definitions (phase 2 output).
// Parentheses and semicolons do not appear: only meaningful constructs.
#ifndef AST_H
#define AST_H

#include <iostream>
#include <string>
#include <vector>
#include "types.h"

enum class NodeKind {
    Program, Block, VarDecl, Assign, If, While, Print,
    Binary, Unary, IntLit, FloatLit, BoolLit, Ident
};

struct Node {
    NodeKind kind;
    int line;
    Type type;          // value type of an expression; filled by semantic analysis
    Node(NodeKind k, int l) : kind(k), line(l), type(TY_ERROR) {}
    virtual ~Node() {}
    virtual void print(std::ostream& os, int indent) const = 0;
};

// Program and Block both hold a list of statements. A Program is the global scope.
struct BlockNode : Node {
    std::vector<Node*> stmts;
    BlockNode(NodeKind k, int l) : Node(k, l) {}
    void print(std::ostream& os, int indent) const;
};

struct VarDeclNode : Node {
    Type declType;
    std::string name;
    std::string uname;  // unique TAC name (set by semantic analysis)
    Node* init;         // may be null
    VarDeclNode(Type t, const std::string& n, Node* i, int l)
        : Node(NodeKind::VarDecl, l), declType(t), name(n), init(i) {}
    void print(std::ostream& os, int indent) const;
};

struct AssignNode : Node {
    std::string name, uname;
    Node* value;
    AssignNode(const std::string& n, Node* v, int l)
        : Node(NodeKind::Assign, l), name(n), value(v) {}
    void print(std::ostream& os, int indent) const;
};

struct IfNode : Node {
    Node *cond, *thenStmt, *elseStmt;  // elseStmt may be null
    IfNode(Node* c, Node* t, Node* e, int l)
        : Node(NodeKind::If, l), cond(c), thenStmt(t), elseStmt(e) {}
    void print(std::ostream& os, int indent) const;
};

struct WhileNode : Node {
    Node *cond, *body;
    WhileNode(Node* c, Node* b, int l) : Node(NodeKind::While, l), cond(c), body(b) {}
    void print(std::ostream& os, int indent) const;
};

struct PrintNode : Node {
    Node* expr;
    PrintNode(Node* e, int l) : Node(NodeKind::Print, l), expr(e) {}
    void print(std::ostream& os, int indent) const;
};

struct BinaryNode : Node {
    std::string op;
    Node *left, *right;
    BinaryNode(const std::string& o, Node* a, Node* b, int l)
        : Node(NodeKind::Binary, l), op(o), left(a), right(b) {}
    void print(std::ostream& os, int indent) const;
};

struct UnaryNode : Node {
    std::string op;     // "!" or "-"
    Node* operand;
    UnaryNode(const std::string& o, Node* e, int l)
        : Node(NodeKind::Unary, l), op(o), operand(e) {}
    void print(std::ostream& os, int indent) const;
};

struct IntLitNode : Node {
    long value;
    IntLitNode(long v, int l) : Node(NodeKind::IntLit, l), value(v) {}
    void print(std::ostream& os, int indent) const;
};

struct FloatLitNode : Node {
    double value;
    FloatLitNode(double v, int l) : Node(NodeKind::FloatLit, l), value(v) {}
    void print(std::ostream& os, int indent) const;
};

struct BoolLitNode : Node {
    bool value;
    BoolLitNode(bool v, int l) : Node(NodeKind::BoolLit, l), value(v) {}
    void print(std::ostream& os, int indent) const;
};

struct IdentNode : Node {
    std::string name, uname;
    IdentNode(const std::string& n, int l) : Node(NodeKind::Ident, l), name(n) {}
    void print(std::ostream& os, int indent) const;
};

#endif
