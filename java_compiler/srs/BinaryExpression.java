public class BinaryExpression extends Expression {
    private final Expression left;
    private final String operator;
    private final Expression right;

    public BinaryExpression(
            Expression left,
            String operator,
            Expression right,
            int line) {

        super(line);
        this.left = left;
        this.operator = operator;
        this.right = right;
    }

    public Expression getLeft() {
        return left;
    }

    public String getOperator() {
        return operator;
    }

    public Expression getRight() {
        return right;
    }

    @Override
    public String print() {
        return "(" + operator + " "
                + left.print() + " "
                + right.print() + ")";
    }
}
