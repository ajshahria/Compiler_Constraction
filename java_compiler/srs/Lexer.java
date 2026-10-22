import java.util.ArrayList;
import java.util.List;

public class Lexer {
    private final String input;
    private int position;
    private int line;

    public Lexer(String input) {
        this.input = input;
        this.position = 0;
        this.line = 1;
    }

    private boolean isAtEnd() {
        return position >= input.length();
    }

    private char peek() {
        if (isAtEnd()) return '\0';
        return input.charAt(position);
    }

    private char advance() {
        char current = input.charAt(position);
        position++;
        return current;
    }

    private boolean isLetter(char c) {
        return Character.isLetter(c) || c == '_';
    }

    private boolean isDigit(char c) {
        return Character.isDigit(c);
    }

    private TokenType keywordType(String word) {
        switch (word) {
            case "int": return TokenType.INT;
            case "float": return TokenType.FLOAT;
            case "char": return TokenType.CHAR;
            case "if": return TokenType.IF;
            case "else": return TokenType.ELSE;
            case "while": return TokenType.WHILE;
            case "print": return TokenType.PRINT;
            default: return TokenType.IDENTIFIER;
        }
    }

    private Token readIdentifier() {
        int start = position;

        while (!isAtEnd() && (isLetter(peek()) || isDigit(peek()))) {
            advance();
        }

        String word = input.substring(start, position);
        return new Token(keywordType(word), word, line);
    }

    private Token readNumber() {
        int start = position;

        while (!isAtEnd() && isDigit(peek())) {
            advance();
        }

        boolean isFloat = false;

        if (!isAtEnd()
                && peek() == '.'
                && position + 1 < input.length()
                && isDigit(input.charAt(position + 1))) {

            isFloat = true;
            advance();

            while (!isAtEnd() && isDigit(peek())) {
                advance();
            }
        }

        String number = input.substring(start, position);

        if (isFloat) {
            return new Token(TokenType.FLOAT_LITERAL, number, line);
        }

        return new Token(TokenType.INTEGER_LITERAL, number, line);
    }

    private Token readChar() {
        int startLine = line;

        advance(); // opening '

        if (isAtEnd() || peek() == '\n') {
            throw new CompilerException(
                    "Lexical Analysis",
                    startLine,
                    "Unterminated character literal"
            );
        }

        char value = advance();

        if (isAtEnd() || peek() != '\'') {
            throw new CompilerException(
                    "Lexical Analysis",
                    startLine,
                    "Invalid character literal"
            );
        }

        advance(); // closing '

        return new Token(
                TokenType.CHAR_LITERAL,
                String.valueOf(value),
                startLine
        );
    }

    public List<Token> tokenize() {
        List<Token> tokens = new ArrayList<>();

        while (!isAtEnd()) {
            char current = peek();

            if (current == ' ' || current == '\t' || current == '\r') {
                advance();
                continue;
            }

            if (current == '\n') {
                line++;
                advance();
                continue;
            }

            if (isLetter(current)) {
                tokens.add(readIdentifier());
                continue;
            }

            if (isDigit(current)) {
                tokens.add(readNumber());
                continue;
            }

            if (current == '\'') {
                tokens.add(readChar());
                continue;
            }

            switch (current) {
                case '+':
                    tokens.add(new Token(TokenType.PLUS, "+", line));
                    advance();
                    break;

                case '-':
                    tokens.add(new Token(TokenType.MINUS, "-", line));
                    advance();
                    break;

                case '*':
                    tokens.add(new Token(TokenType.MULTIPLY, "*", line));
                    advance();
                    break;

                case '%':
                    tokens.add(new Token(TokenType.MODULO, "%", line));
                    advance();
                    break;

                case '(':
                    tokens.add(new Token(TokenType.LEFT_PAREN, "(", line));
                    advance();
                    break;

                case ')':
                    tokens.add(new Token(TokenType.RIGHT_PAREN, ")", line));
                    advance();
                    break;

                case '{':
                    tokens.add(new Token(TokenType.LEFT_BRACE, "{", line));
                    advance();
                    break;

                case '}':
                    tokens.add(new Token(TokenType.RIGHT_BRACE, "}", line));
                    advance();
                    break;

                case ';':
                    tokens.add(new Token(TokenType.SEMICOLON, ";", line));
                    advance();
                    break;

                case '/':
                    advance();

                    if (!isAtEnd() && peek() == '/') {
                        while (!isAtEnd() && peek() != '\n') {
                            advance();
                        }
                    } else {
                        tokens.add(new Token(TokenType.DIVIDE, "/", line));
                    }
                    break;

                case '=':
                    advance();

                    if (!isAtEnd() && peek() == '=') {
                        advance();
                        tokens.add(new Token(TokenType.EQUAL, "==", line));
                    } else {
                        tokens.add(new Token(TokenType.ASSIGN, "=", line));
                    }
                    break;

                case '<':
                    advance();

                    if (!isAtEnd() && peek() == '=') {
                        advance();
                        tokens.add(new Token(TokenType.LESS_EQUAL, "<=", line));
                    } else {
                        tokens.add(new Token(TokenType.LESS, "<", line));
                    }
                    break;

                case '>':
                    advance();

                    if (!isAtEnd() && peek() == '=') {
                        advance();
                        tokens.add(new Token(TokenType.GREATER_EQUAL, ">=", line));
                    } else {
                        tokens.add(new Token(TokenType.GREATER, ">", line));
                    }
                    break;

                case '!':
                    advance();

                    if (!isAtEnd() && peek() == '=') {
                        advance();
                        tokens.add(new Token(TokenType.NOT_EQUAL, "!=", line));
                    } else {
                        throw new CompilerException(
                                "Lexical Analysis",
                                line,
                                "Unexpected character '!'"
                        );
                    }
                    break;

                default:
                    throw new CompilerException(
                            "Lexical Analysis",
                            line,
                            "Unexpected character '" + current + "'"
                    );
            }
        }

        tokens.add(new Token(TokenType.EOF, "", line));
        return tokens;
    }
}

