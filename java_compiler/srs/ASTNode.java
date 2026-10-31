public abstract class ASTNode {
    private final int line;

    public ASTNode() {
        this.line = 0;
    }

    public ASTNode(int line) {
        this.line = line;
    }

    public int getLine() {
        return line;
    }

    public abstract String print();
}
