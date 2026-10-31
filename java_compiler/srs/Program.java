import java.util.List;

public class Program extends ASTNode {
    private final List<Statement> statements;

    public Program(List<Statement> statements) {
        super(0);
        this.statements = statements;
    }

    public List<Statement> getStatements() {
        return statements;
    }

    @Override
    public String print() {
        StringBuilder result = new StringBuilder();

        for (Statement statement : statements) {
            result.append(statement.print()).append("\n");
        }

        return result.toString();
    }
}

