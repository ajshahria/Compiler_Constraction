import java.util.ArrayList;
import java.util.List;

public class Parser {
    private final List<Token> tokens;
    private int current;

    public Parser(List<Token> tokens) {
        this.tokens = tokens;
        this.current = 0;
    }

    public Program parse() {
        List<Statement> statements = new ArrayList<>();

        while (!check(TokenType.EOF)) {
            statements.add(statement());
        }

        return new Program(statements);
    }

    public Expression parseExpression() {
        Expression expression = expression();

        if (!check(TokenType.EOF)) {
            throw new CompilerException(
                    "Syntax Analysis",
                    peek().getLine(),
                    "Unexpected token '" + peek().getValue() + "'"
            );
        }

        return expression;
    }

    private Statement statement() {
        if (match(TokenType.INT, TokenType.FLOAT, TokenType.CHAR)) {
            return declaration(previous());
        }

        if (match(TokenType.IF)) {
            return ifStatement(previous());
        }

        if (match(TokenType.WHILE)) {
            return whileStatement(previous());
        }

        if (match(TokenType.PRINT)) {
            return printStatement(previous());
        }

        if (match(TokenType.LEFT_BRACE)) {
            return block(previous());
        }

        if (check(TokenType.IDENTIFIER)) {
            return assignment();
        }

        throw new CompilerException(
                "Syntax Analysis",
                peek().getLine(),
                "Unexpected token '" + peek().getValue() + "'"
        );
    }

    private Declaration declaration(Token typeToken) {
        Token name = consume(
                TokenType.IDENTIFIER,
                "Expected identifier after type"
        );

        Expression initializer = null;

        if (match(TokenType.ASSIGN)) {
            initializer = expression();
        }

        consume(
                TokenType.SEMICOLON,
                "Expected ';' after declaration"
        );

        return new Declaration(
                typeToken.getValue(),
                name.getValue(),
                initializer,
                typeToken.getLine()
        );
    }

    private Assignment assignment() {
        Token name = consume(
                TokenType.IDENTIFIER,
                "Expected identifier"
        );

        consume(
                TokenType.ASSIGN,
                "Expected '=' after identifier"
        );

        Expression value = expression();

        consume(
                TokenType.SEMICOLON,
                "Expected ';' after assignment"
        );

        return new Assignment(
                name.getValue(),
                value,
                name.getLine()
        );
    }

    private PrintStatement printStatement(Token printToken) {
        consume(
                TokenType.LEFT_PAREN,
                "Expected '(' after print"
        );

        Expression expression = expression();

        consume(
                TokenType.RIGHT_PAREN,
                "Expected ')' after print expression"
        );

        consume(
                TokenType.SEMICOLON,
                "Expected ';' after print statement"
        );

        return new PrintStatement(
                expression,
                printToken.getLine()
        );
    }

    private IfStatement ifStatement(Token ifToken) {
        consume(
                TokenType.LEFT_PAREN,
                "Expected '(' after if"
        );

        Expression condition = expression();

        consume(
                TokenType.RIGHT_PAREN,
                "Expected ')' after if condition"
        );

        if (!check(TokenType.LEFT_BRACE)) {
            throw new CompilerException(
                    "Syntax Analysis",
                    peek().getLine(),
                    "Expected '{' after if condition"
            );
        }

        Token thenBrace = advance();
        Block thenBranch = block(thenBrace);

        Block elseBranch = null;

        if (match(TokenType.ELSE)) {
            if (!check(TokenType.LEFT_BRACE)) {
                throw new CompilerException(
                        "Syntax Analysis",
                        peek().getLine(),
                        "Expected '{' after else"
                );
            }

            Token elseBrace = advance();
            elseBranch = block(elseBrace);
        }

        return new IfStatement(
                condition,
                thenBranch,
                elseBranch,
                ifToken.getLine()
        );
    }

    private WhileStatement whileStatement(Token whileToken) {
        consume(
                TokenType.LEFT_PAREN,
                "Expected '(' after while"
        );

        Expression condition = expression();

        consume(
                TokenType.RIGHT_PAREN,
                "Expected ')' after while condition"
        );

        if (!check(TokenType.LEFT_BRACE)) {
            throw new CompilerException(
                    "Syntax Analysis",
                    peek().getLine(),
                    "Expected '{' after while condition"
            );
        }

        Token brace = advance();
        Block body = block(brace);

        return new WhileStatement(
                condition,
                body,
                whileToken.getLine()
        );
    }

    private Block block(Token braceToken) {
        List<Statement> statements = new ArrayList<>();

        while (!check(TokenType.RIGHT_BRACE)
                && !check(TokenType.EOF)) {

            statements.add(statement());
        }

        consume(
                TokenType.RIGHT_BRACE,
                "Expected '}' after block"
        );

        return new Block(
                statements,
                braceToken.getLine()
        );
    }

    private Expression expression() {
        return equality();
    }

    private Expression equality() {
        Expression expr = comparison();

        while (match(TokenType.EQUAL, TokenType.NOT_EQUAL)) {
            Token operator = previous();
            Expression right = comparison();

            expr = new BinaryExpression(
                    expr,
                    operator.getValue(),
                    right,
                    operator.getLine()
            );
        }

        return expr;
    }

    private Expression comparison() {
        Expression expr = term();

        while (match(
                TokenType.LESS,
                TokenType.GREATER,
                TokenType.LESS_EQUAL,
                TokenType.GREATER_EQUAL)) {

            Token operator = previous();
            Expression right = term();

            expr = new BinaryExpression(
                    expr,
                    operator.getValue(),
                    right,
                    operator.getLine()
            );
        }

        return expr;
    }

    private Expression term() {
        Expression expr = factor();

        while (match(TokenType.PLUS, TokenType.MINUS)) {
            Token operator = previous();
            Expression right = factor();

            expr = new BinaryExpression(
                    expr,
                    operator.getValue(),
                    right,
                    operator.getLine()
            );
        }

        return expr;
    }

    private Expression factor() {
        Expression expr = unary();

        while (match(
                TokenType.MULTIPLY,
                TokenType.DIVIDE,
                TokenType.MODULO)) {

            Token operator = previous();
            Expression right = unary();

            expr = new BinaryExpression(
                    expr,
                    operator.getValue(),
                    right,
                    operator.getLine()
            );
        }

        return expr;
    }

    private Expression unary() {
        if (match(TokenType.MINUS)) {
            Token operator = previous();
            Expression right = unary();

            return new BinaryExpression(
                    new Literal("0", "int", operator.getLine()),
                    "-",
                    right,
                    operator.getLine()
            );
        }

        return primary();
    }

    private Expression primary() {
        if (match(TokenType.INTEGER_LITERAL)) {
            Token token = previous();
            return new Literal(
                    token.getValue(),
                    "int",
                    token.getLine()
            );
        }

        if (match(TokenType.FLOAT_LITERAL)) {
            Token token = previous();
            return new Literal(
                    token.getValue(),
                    "float",
                    token.getLine()
            );
        }

        if (match(TokenType.CHAR_LITERAL)) {
            Token token = previous();
            return new Literal(
                    token.getValue(),
                    "char",
                    token.getLine()
            );
        }

        if (match(TokenType.IDENTIFIER)) {
            Token token = previous();
            return new Variable(
                    token.getValue(),
                    token.getLine()
            );
        }

        if (match(TokenType.LEFT_PAREN)) {
            Expression expr = expression();

            consume(
                    TokenType.RIGHT_PAREN,
                    "Expected ')' after expression"
            );

            return expr;
        }

        throw new CompilerException(
                "Syntax Analysis",
                peek().getLine(),
                "Expected expression"
        );
    }

    private boolean match(TokenType... types) {
        for (TokenType type : types) {
            if (check(type)) {
                advance();
                return true;
            }
        }

        return false;
    }

    private Token consume(TokenType type, String message) {
        if (check(type)) {
            return advance();
        }

        throw new CompilerException(
                "Syntax Analysis",
                peek().getLine(),
                message
        );
    }

    private boolean check(TokenType type) {
        return peek().getType() == type;
    }

    private Token advance() {
        if (!isAtEnd()) {
            current++;
        }

        return previous();
    }

    private boolean isAtEnd() {
        return peek().getType() == TokenType.EOF;
    }

    private Token peek() {
        return tokens.get(current);
    }

    private Token previous() {
        return tokens.get(current - 1);
    }
}

