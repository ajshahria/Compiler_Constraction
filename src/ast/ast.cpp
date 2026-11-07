// ast.cpp - indented text printer for the AST.
#include "ast.h"

static void pad(std::ostream& os, int n) {
    for (int i = 0; i < n; i++) os << "  ";
}

void BlockNode::print(std::ostream& os, int indent) const {
    pad(os, indent);
    os << (kind == NodeKind::Program ? "Program" : "Block") << "\n";
    for (size_t i = 0; i < stmts.size(); i++) stmts[i]->print(os, indent + 1);
}

void VarDeclNode::print(std::ostream& os, int indent) const {
    pad(os, indent);
    os << "VarDecl " << typeName(declType) << " " << name << " (line " << line << ")\n";
    if (init) init->print(os, indent + 1);
}

void AssignNode::print(std::ostream& os, int indent) const {
    pad(os, indent);
    os << "Assign " << name << " (line " << line << ")\n";
    value->print(os, indent + 1);
}

void IfNode::print(std::ostream& os, int indent) const {
    pad(os, indent);
    os << "If (line " << line << ")\n";
    pad(os, indent + 1); os << "Condition:\n";
    cond->print(os, indent + 2);
    pad(os, indent + 1); os << "Then:\n";
    thenStmt->print(os, indent + 2);
    if (elseStmt) {
        pad(os, indent + 1); os << "Else:\n";
        elseStmt->print(os, indent + 2);
    }
}

void WhileNode::print(std::ostream& os, int indent) const {
    pad(os, indent);
    os << "While (line " << line << ")\n";
    pad(os, indent + 1); os << "Condition:\n";
    cond->print(os, indent + 2);
    pad(os, indent + 1); os << "Body:\n";
    body->print(os, indent + 2);
}

void PrintNode::print(std::ostream& os, int indent) const {
    pad(os, indent);
    os << "Print (line " << line << ")\n";
    expr->print(os, indent + 1);
}

void BinaryNode::print(std::ostream& os, int indent) const {
    pad(os, indent);
    os << "BinaryOp '" << op << "'\n";
    left->print(os, indent + 1);
    right->print(os, indent + 1);
}

void UnaryNode::print(std::ostream& os, int indent) const {
    pad(os, indent);
    os << "UnaryOp '" << op << "'\n";
    operand->print(os, indent + 1);
}

void IntLitNode::print(std::ostream& os, int indent) const {
    pad(os, indent); os << "IntLit " << value << "\n";
}

void FloatLitNode::print(std::ostream& os, int indent) const {
    pad(os, indent); os << "FloatLit " << value << "\n";
}

void BoolLitNode::print(std::ostream& os, int indent) const {
    pad(os, indent); os << "BoolLit " << (value ? "true" : "false") << "\n";
}

void IdentNode::print(std::ostream& os, int indent) const {
    pad(os, indent); os << "Ident " << name << "\n";
}
