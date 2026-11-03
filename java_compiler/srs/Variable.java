public class Variable extends Expression {
    private final String name;

    public Variable(String name, int line) {
        super(line);
        this.name = name;
    }

    public String getName() {
        return name;
    }

    @Override
    public String print() {
        return name;
    }
}
